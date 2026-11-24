#include <functional>
#include <Beam/Queues/Queue.hpp>
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterController.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/FlattenController.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace Spire;

namespace {
  struct SubmissionState {
    bool m_is_failure;
    std::function<void ()> m_on_submit;

    SubmissionState()
      : m_is_failure(false) {}
  };

  class SubmissionClient : public OrderExecutionClient {
    public:
      std::shared_ptr<SubmissionState> m_state;

      SubmissionClient(OrderExecutionClient client,
        std::shared_ptr<SubmissionState> state)
        : OrderExecutionClient(std::move(client)),
          m_state(std::move(state)) {}

      std::shared_ptr<Order> submit(const OrderFields& fields) {
        if(std::exchange(m_state->m_is_failure, false)) {
          throw std::runtime_error("Submission failed.");
        }
        if(m_state->m_on_submit) {
          m_state->m_on_submit();
        }
        return OrderExecutionClient::submit(fields);
      }
  };

  class SubmissionClients : public TestClients {
    public:
      OrderExecutionClient m_client;

      SubmissionClients(Ref<TestEnvironment> environment,
        std::shared_ptr<SubmissionState> state)
        : TestClients(environment),
          m_client(std::in_place_type<SubmissionClient>,
            TestClients::get_order_execution_client(), std::move(state)) {}

      OrderExecutionClient& get_order_execution_client() {
        return m_client;
      }
  };

  struct Blotter {
    std::shared_ptr<ArrayListModel<BlotterTaskEntry>> m_tasks;
    std::shared_ptr<ArrayListModel<std::shared_ptr<Order>>> m_orders;
    std::shared_ptr<BlotterModel> m_model;
    std::unique_ptr<BlotterController> m_controller;

    Blotter(UserProfile& profile,
        const std::shared_ptr<BlotterCollection>& collection,
        const DirectoryEntry& account, bool is_consolidated)
        : m_tasks(std::make_shared<ArrayListModel<BlotterTaskEntry>>()),
          m_orders(std::make_shared<ArrayListModel<std::shared_ptr<Order>>>()),
          m_model(collection->add(std::make_shared<CompositeBlotterModel>(
            account, is_consolidated, std::make_shared<LocalTextModel>("Test"),
            std::make_shared<LocalBooleanModel>(false),
            std::make_shared<LocalBooleanModel>(false), m_tasks,
            std::make_shared<ArrayListModel<BlotterTaskMonitor>>(), m_orders,
            std::make_shared<ArrayListModel<PortfolioUpdateEntry>>()))),
          m_controller(std::make_unique<BlotterController>(m_model, m_tasks,
            collection, std::make_shared<CancelKeyBindingsModel>(),
            std::make_shared<PositionKeyBindingsModel>(), Ref(profile))) {}
  };

  struct Fixture {
    TestEnvironment m_environment;
    std::shared_ptr<SubmissionState> m_submission;
    UserProfile m_profile;
    OrderExecutionClient m_client;
    std::shared_ptr<BlotterCollection> m_collection;
    std::unique_ptr<FlattenController> m_controller;
    std::shared_ptr<Queue<std::shared_ptr<Order>>> m_submissions;
    std::vector<std::unique_ptr<Blotter>> m_blotters;
    std::vector<std::weak_ptr<Task>> m_tasks;

    Fixture()
        : m_environment(time_from_string("2026-11-23 15:00:00")),
          m_submission(std::make_shared<SubmissionState>()),
          m_profile("", false, false, {}, {},
            get_default_additional_tag_database(), {}, {}, {},
            Clients(std::in_place_type<SubmissionClients>, Ref(m_environment),
              m_submission)),
          m_client(m_profile.GetClients().get_order_execution_client()),
          m_collection(std::make_shared<BlotterCollection>(
            m_profile.GetClients().get_market_data_client())),
          m_controller(std::make_unique<FlattenController>(
            Ref(m_profile), m_collection)),
          m_submissions(std::make_shared<Queue<std::shared_ptr<Order>>>()) {
      initialize_resources();
      m_environment.monitor_order_submissions(m_submissions);
      m_environment.publish(parse_ticker("RY.TSX"),
        BboQuote(make_bid(Money(10), 100), make_ask(Money(11), 100),
          time_from_string("2026-11-23 15:00:00")));
    }

    ~Fixture() {
      for(auto& weak : m_tasks) {
        if(auto task = weak.lock()) {
          task->GetExecutor().Close();
        }
      }
      for(auto& blotter : m_blotters) {
        for(auto& entry : std::as_const(*blotter->m_tasks)) {
          entry.m_task->GetExecutor().Close();
        }
      }
      m_controller.reset();
      flush_pending_routines();
    }

    auto get_account() {
      return m_profile.GetClients().get_service_locator_client().get_account();
    }

    Blotter& add(const DirectoryEntry& account, bool is_consolidated) {
      m_blotters.push_back(std::make_unique<Blotter>(
        m_profile, m_collection, account, is_consolidated));
      return *m_blotters.back();
    }

    Blotter& add(bool is_consolidated) {
      return add(get_account(), is_consolidated);
    }

    auto pop() {
      return std::dynamic_pointer_cast<PrimitiveOrder>(m_submissions->pop());
    }

    void update(PrimitiveOrder& order, const auto& action) {
      auto reports = std::make_shared<Queue<ExecutionReport>>();
      m_client.load_order(order.get_info().m_id)->get_publisher().monitor(
        reports);
      action();
      auto sequence = order.get_publisher().get_snapshot()->back().m_sequence;
      while(reports->pop().m_sequence != sequence) {}
    }

    auto trade(Blotter& blotter, Quantity quantity) {
      auto order = m_client.submit(make_limit_order_fields(
        blotter.m_model->get_account(), parse_ticker("RY.TSX"), Currencies::CAD,
        get_side(quantity), "TSX", abs(quantity), Money(10)));
      auto submission = pop();
      blotter.m_orders->push(order);
      update(*submission, [&] {
        fill(*submission, abs(quantity));
      });
      return submission;
    }

    void flatten(Blotter& blotter) {
      m_controller->flatten(*blotter.m_controller, parse_ticker("RY.TSX"));
      for(auto& entry : std::as_const(*blotter.m_tasks)) {
        m_tasks.emplace_back(entry.m_task);
      }
    }

    void request_cancel(PrimitiveOrder& order) {
      auto reports = std::make_shared<Queue<ExecutionReport>>();
      m_client.load_order(order.get_info().m_id)->get_publisher().monitor(
        reports);
      m_client.cancel(order);
      while(reports->pop().m_status != OrderStatus::PENDING_CANCEL) {}
    }
  };

  void require_order(const Order& order,
      const DirectoryEntry& account, Quantity quantity) {
    auto& fields = order.get_info().m_fields;
    REQUIRE(fields.m_account == account);
    REQUIRE(fields.m_ticker == parse_ticker("RY.TSX"));
    REQUIRE(fields.m_currency == Currencies::CAD);
    REQUIRE(fields.m_type == OrderType::MARKET);
    REQUIRE(fields.m_side == get_side(quantity));
    REQUIRE(fields.m_quantity == abs(quantity));
    REQUIRE(fields.m_destination == "TSX");
  }
}

TEST_SUITE("FlattenController") {
  UI_TEST_CASE("repeated_requests_and_partial_fills") {
    auto fixture = Fixture();
    auto& blotter = fixture.add(false);
    auto quantity = Quantity(100);
    SUBCASE("long") {}
    SUBCASE("short") {
      quantity = -100;
    }
    fixture.flatten(blotter);
    REQUIRE(blotter.m_tasks->get_size() == 0);
    fixture.trade(blotter, quantity);
    fixture.flatten(blotter);
    auto order = fixture.pop();
    require_order(*order, fixture.get_account(), -quantity);
    fixture.flatten(blotter);
    REQUIRE(blotter.m_tasks->get_size() == 1);
    fixture.update(*order, [&] {
      fill(*order, Money(10), 30);
    });
    fixture.flatten(blotter);
    REQUIRE(blotter.m_tasks->get_size() == 1);
    fixture.update(*order, [&] {
      fill(*order, Money(10), 70);
    });
    fixture.flatten(blotter);
    REQUIRE(blotter.m_tasks->get_size() == 1);
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("intervening_trades") {
    auto fixture = Fixture();
    auto& blotter = fixture.add(false);
    fixture.trade(blotter, 100);
    fixture.flatten(blotter);
    auto first = fixture.pop();
    auto change = Quantity(40);
    SUBCASE("increase") {}
    SUBCASE("decrease") {
      change = -40;
    }
    fixture.trade(blotter, change);
    fixture.flatten(blotter);
    auto second = fixture.pop();
    require_order(*first, fixture.get_account(), -100);
    require_order(*second, fixture.get_account(), -change);
    REQUIRE(blotter.m_tasks->get_size() == 2);
    fixture.flatten(blotter);
    REQUIRE(blotter.m_tasks->get_size() == 2);
    fixture.update(*second, [&] {
      fill(*second, Money(10), 40);
    });
    fixture.flatten(blotter);
    REQUIRE(blotter.m_tasks->get_size() == 2);
    fixture.update(*first, [&] {
      fill(*first, Money(10), 100);
    });
    fixture.flatten(blotter);
    REQUIRE(blotter.m_tasks->get_size() == 2);
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("cancellation_and_rejection") {
    auto fixture = Fixture();
    auto& blotter = fixture.add(false);
    fixture.trade(blotter, 100);
    fixture.flatten(blotter);
    auto order = fixture.pop();
    auto remaining = Quantity(100);
    SUBCASE("rejection") {
      fixture.update(*order, [&] {
        reject(*order);
      });
    }
    SUBCASE("cancellation") {
      fixture.update(*order, [&] {
        fill(*order, Money(10), 30);
      });
      fixture.request_cancel(*order);
      fixture.flatten(blotter);
      REQUIRE(blotter.m_tasks->get_size() == 1);
      fixture.update(*order, [&] {
        fill(*order, Money(10), 20);
      });
      fixture.flatten(blotter);
      REQUIRE(blotter.m_tasks->get_size() == 1);
      fixture.update(*order, [&] {
        cancel(*order);
      });
      remaining = 50;
    }
    fixture.flatten(blotter);
    require_order(*fixture.pop(), fixture.get_account(), -remaining);
    REQUIRE(blotter.m_tasks->get_size() == 2);
  }

  UI_TEST_CASE("submission_failure") {
    auto fixture = Fixture();
    auto& blotter = fixture.add(false);
    fixture.trade(blotter, 100);
    fixture.m_submission->m_is_failure = true;
    REQUIRE_THROWS_AS(fixture.flatten(blotter), std::runtime_error);
    REQUIRE(blotter.m_tasks->get_size() == 0);
    REQUIRE(!fixture.m_submissions->try_pop());
    fixture.flatten(blotter);
    require_order(*fixture.pop(), fixture.get_account(), -100);
    fixture.flatten(blotter);
    REQUIRE(blotter.m_tasks->get_size() == 1);
  }

  UI_TEST_CASE("reentrant_requests") {
    auto fixture = Fixture();
    auto& blotter = fixture.add(false);
    fixture.trade(blotter, 100);
    auto requests = 0;
    fixture.m_submission->m_on_submit = [&] {
      ++requests;
      fixture.flatten(blotter);
    };
    auto connection = scoped_connection(
      blotter.m_tasks->connect_operation_signal([&] (const auto& operation) {
        if(std::holds_alternative<TasksModel::AddOperation>(operation)) {
          ++requests;
          fixture.flatten(blotter);
        }
      }));
    fixture.flatten(blotter);
    REQUIRE(requests == 2);
    require_order(*fixture.pop(), fixture.get_account(), -100);
    REQUIRE(blotter.m_tasks->get_size() == 1);
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("fill_before_task_publication") {
    auto fixture = Fixture();
    auto& blotter = fixture.add(false);
    fixture.trade(blotter, 100);
    auto order = std::shared_ptr<PrimitiveOrder>();
    auto connection = scoped_connection(
      blotter.m_tasks->connect_operation_signal([&] (const auto& operation) {
        if(std::holds_alternative<TasksModel::AddOperation>(operation)) {
          order = fixture.pop();
          fixture.update(*order, [&] {
            fill(*order, Money(10), 30);
          });
          fixture.flatten(blotter);
        }
      }));
    fixture.flatten(blotter);
    REQUIRE(order);
    require_order(*order, fixture.get_account(), -100);
    REQUIRE(blotter.m_tasks->get_size() == 1);
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("replayed_reports") {
    auto fixture = Fixture();
    auto& blotter = fixture.add(false);
    auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
      make_limit_order_fields(fixture.get_account(), parse_ticker("RY.TSX"),
        Currencies::CAD, Side::BID, "TSX", 200, Money(10)), 999999,
      time_from_string("2026-11-23 15:00:00")));
    auto report = fill(*order, 100);
    order->update(report);
    blotter.m_orders->push(order);
    blotter.m_orders->push(order);
    fixture.flatten(blotter);
    require_order(*fixture.pop(), fixture.get_account(), -100);
    fixture.flatten(blotter);
    REQUIRE(blotter.m_tasks->get_size() == 1);
    report = make_update(report, OrderStatus::PARTIALLY_FILLED,
      time_from_string("2026-11-23 15:00:01"));
    report.m_last_quantity = 25;
    report.m_last_price = Money(10);
    order->update(report);
    order->update(report);
    fixture.flatten(blotter);
    require_order(*fixture.pop(), fixture.get_account(), -25);
    REQUIRE(blotter.m_tasks->get_size() == 2);
  }

  UI_TEST_CASE("snapshot_excluded_order") {
    auto fixture = Fixture();
    auto& blotter = fixture.add(true);
    auto order = fixture.m_client.submit(make_limit_order_fields(
      fixture.get_account(), parse_ticker("RY.TSX"), Currencies::CAD,
      Side::BID, "TSX", 100, Money(10)));
    auto submission = fixture.pop();
    blotter.m_orders->push(order);
    fixture.update(*submission, [&] {
      fill(*submission, 30);
    });
    flush_pending_routines();
    fixture.flatten(blotter);
    require_order(*fixture.pop(), fixture.get_account(), -30);
    fixture.update(*submission, [&] {
      fill(*submission, 20);
    });
    fixture.flatten(blotter);
    require_order(*fixture.pop(), fixture.get_account(), -20);
    REQUIRE(blotter.m_tasks->get_size() == 2);
  }

  UI_TEST_CASE("account_snapshots_and_live_orders") {
    auto fixture = Fixture();
    auto& blotter = fixture.add(true);
    fixture.trade(blotter, 100);
    flush_pending_routines();
    auto snapshot = fixture.m_profile.GetClients().get_risk_client().
      load_inventory_snapshot(fixture.get_account());
    REQUIRE(snapshot.m_inventories.size() == 1);
    REQUIRE(snapshot.m_inventories.front().m_position.m_quantity == 100);
    fixture.flatten(blotter);
    auto first = fixture.pop();
    require_order(*first, fixture.get_account(), -100);
    fixture.trade(blotter, -40);
    fixture.flatten(blotter);
    auto second = fixture.pop();
    require_order(*second, fixture.get_account(), 40);
    fixture.update(*first, [&] {
      fill(*first, Money(10), 100);
    });
    fixture.flatten(blotter);
    REQUIRE(blotter.m_tasks->get_size() == 2);
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("account_isolation") {
    auto fixture = Fixture();
    auto account = fixture.m_profile.GetClients().get_service_locator_client().
      make_account("other", "", DirectoryEntry::STAR_DIRECTORY);
    auto& first = fixture.add(true);
    auto& second = fixture.add(account, true);
    fixture.trade(first, 100);
    fixture.trade(second, -200);
    fixture.flatten(first);
    require_order(*fixture.pop(), fixture.get_account(), -100);
    fixture.flatten(second);
    require_order(*fixture.pop(), account, 200);
    fixture.flatten(first);
    fixture.flatten(second);
    REQUIRE(first.m_tasks->get_size() == 1);
    REQUIRE(second.m_tasks->get_size() == 1);
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("linked_scopes") {
    auto fixture = Fixture();
    auto& consolidated = fixture.add(true);
    auto& source = fixture.add(false);
    auto& first = fixture.add(false);
    auto& second = fixture.add(false);
    auto& receiver = fixture.add(false);
    fixture.m_collection->link(*first.m_model, *source.m_model);
    fixture.m_collection->link(*second.m_model, *source.m_model);
    fixture.m_collection->link(*receiver.m_model, *first.m_model);
    fixture.m_collection->link(*receiver.m_model, *second.m_model);
    fixture.trade(source, 100);
    fixture.flatten(source);
    require_order(*fixture.pop(), fixture.get_account(), -100);
    fixture.flatten(receiver);
    fixture.flatten(consolidated);
    REQUIRE(receiver.m_tasks->get_size() == 0);
    REQUIRE(consolidated.m_tasks->get_size() == 0);
    fixture.trade(receiver, 40);
    fixture.flatten(receiver);
    require_order(*fixture.pop(), fixture.get_account(), -40);
    fixture.flatten(consolidated);
    REQUIRE(consolidated.m_tasks->get_size() == 0);
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("removed_controller") {
    auto fixture = Fixture();
    auto& consolidated = fixture.add(true);
    auto& source = fixture.add(false);
    fixture.trade(source, 100);
    fixture.flatten(source);
    auto first = fixture.pop();
    auto task = std::weak_ptr(source.m_tasks->get(0).m_task);
    source.m_controller.reset();
    fixture.m_collection->remove(std::ranges::find(
      std::as_const(*fixture.m_collection), source.m_model));
    clear(*source.m_tasks);
    REQUIRE_FALSE(task.expired());
    fixture.flatten(consolidated);
    REQUIRE(consolidated.m_tasks->get_size() == 0);
    fixture.trade(consolidated, -40);
    fixture.flatten(consolidated);
    require_order(*fixture.pop(), fixture.get_account(), 40);
    fixture.update(*first, [&] {
      fill(*first, Money(10), 100);
    });
    fixture.flatten(consolidated);
    REQUIRE(consolidated.m_tasks->get_size() == 1);
    REQUIRE(!fixture.m_submissions->try_pop());
  }
}
