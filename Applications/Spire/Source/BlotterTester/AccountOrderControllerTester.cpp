#include <Beam/Queues/SequencePublisher.hpp>
#include <QEventLoop>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/AccountOrderController.hpp"
#include "Spire/Blotter/BlotterRegistry.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderWrapperTaskNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  struct Fixture {
    TestEnvironment m_environment;
    UserProfile m_profile;
    std::shared_ptr<Queue<std::shared_ptr<Order>>> m_submissions;

    Fixture()
        : m_environment(time_from_string("2026-11-23 15:00:00")),
          m_profile("", false, false, {}, {},
            get_default_additional_tag_database(), {}, {}, {},
            Clients(std::in_place_type<TestClients>, Ref(m_environment))),
          m_submissions(std::make_shared<Queue<std::shared_ptr<Order>>>()) {
      m_environment.monitor_order_submissions(m_submissions);
    }

    ~Fixture() {
      for(auto& model :
          std::as_const(*m_profile.GetBlotterRegistry().get_blotters())) {
        for(auto& entry : std::as_const(*model->get_tasks())) {
          entry.m_task->GetExecutor().Close();
        }
      }
      flush_pending_routines();
    }

    auto get_account() {
      return m_profile.GetClients().get_service_locator_client().get_account();
    }

    auto submit(const Ticker& ticker, Side side) {
      m_environment.publish(ticker, BboQuote(make_bid(Money(10), 100),
        make_ask(Money(11), 100), time_from_string("2026-11-23 15:00:00")));
      m_profile.GetClients().get_order_execution_client().submit(
        make_limit_order_fields(get_account(), ticker, Currencies::CAD,
          side, "TSX", 100, Money(10)));
      auto order = m_submissions->pop();
      m_environment.accept(*order);
      return order;
    }

    auto submit() {
      return submit(parse_ticker("RY.TSX"), Side::BID);
    }
  };

  class CancellationClient : public OrderExecutionClient {
    public:
      CancellationClient(OrderExecutionClient client,
        std::shared_ptr<Queue<OrderId>> cancellations)
        : OrderExecutionClient(std::move(client)),
          m_cancellations(std::move(cancellations)) {}

      void cancel(const std::shared_ptr<Order>& order) {
        cancel(*order);
      }

      void cancel(const Order& order) {
        m_cancellations->push(order.get_info().m_id);
      }

    private:
      std::shared_ptr<Queue<OrderId>> m_cancellations;
  };

  class CancellationClients : public TestClients {
    public:
      CancellationClients(Ref<TestEnvironment> environment,
        std::shared_ptr<Queue<OrderId>> cancellations)
        : TestClients(environment),
          m_client(std::in_place_type<CancellationClient>,
            TestClients::get_order_execution_client(),
            std::move(cancellations)) {}

      OrderExecutionClient& get_order_execution_client() {
        return m_client;
      }

    private:
      OrderExecutionClient m_client;
  };

  class ReportingOrder : public Order {
    public:
      ReportingOrder(OrderInfo info)
          : m_info(std::move(info)) {
        m_reports.push(ExecutionReport(m_info.m_id, m_info.m_timestamp));
        publish(OrderStatus::NEW);
      }

      void publish(OrderStatus status) {
        auto report = make_update(m_reports.get_snapshot()->back(), status,
          m_info.m_timestamp);
        if(status == OrderStatus::FILLED) {
          report.m_last_quantity = m_info.m_fields.m_quantity;
        }
        m_reports.push(report);
      }

      void replay() {
        m_reports.push(m_reports.get_snapshot()->back());
      }

      const OrderInfo& get_info() const override {
        return m_info;
      }

      const SnapshotPublisher<ExecutionReport,
          std::vector<ExecutionReport>>& get_publisher() const override {
        return m_reports;
      }

    private:
      OrderInfo m_info;
      SequencePublisher<ExecutionReport> m_reports;
  };

  void flush() {
    flush_pending_routines();
    QApplication::sendPostedEvents();
  }

  class ControllerFixture {
    public:
      TestEnvironment m_environment;
      std::shared_ptr<Queue<OrderId>> m_cancellations;
      UserProfile m_profile;
      std::shared_ptr<BlotterCollection> m_collection;
      FlattenController m_flatten_controller;
      std::shared_ptr<ArrayListModel<BlotterTaskEntry>> m_tasks;
      std::shared_ptr<AccountOrdersModel> m_orders;
      std::shared_ptr<BlotterModel> m_model;
      std::unique_ptr<BlotterController> m_blotter;
      std::unique_ptr<AccountOrderController> m_controller;
      std::vector<std::shared_ptr<Task>> m_owned_tasks;
      OrderId m_next_id;

      ControllerFixture()
        : ControllerFixture(false) {}

      ControllerFixture(bool is_other_account)
          : m_environment(time_from_string("2026-11-23 15:00:00")),
            m_cancellations(std::make_shared<Queue<OrderId>>()),
            m_profile("", false, false, {}, {},
              get_default_additional_tag_database(), {}, {}, {},
              Clients(std::in_place_type<CancellationClients>,
                Ref(m_environment), m_cancellations)),
            m_collection(std::make_shared<BlotterCollection>(
              m_profile.GetClients().get_market_data_client())),
            m_flatten_controller(Ref(m_profile), m_collection),
            m_tasks(std::make_shared<ArrayListModel<BlotterTaskEntry>>()),
            m_next_id(1) {
        auto account =
          m_profile.GetClients().get_service_locator_client().get_account();
        if(is_other_account) {
          account = m_profile.GetClients().get_service_locator_client().
            make_account("other", "", DirectoryEntry::STAR_DIRECTORY);
        }
        m_orders =
          std::make_shared<AccountOrdersModel>(account, m_profile.GetClients());
        m_model = std::make_shared<CompositeBlotterModel>(account, true,
          std::make_shared<LocalTextModel>("Test"),
          std::make_shared<LocalBooleanModel>(false),
          std::make_shared<LocalBooleanModel>(false), m_tasks,
          std::make_shared<ArrayListModel<BlotterTaskMonitor>>(), m_orders,
          std::make_shared<ArrayListModel<PortfolioUpdateEntry>>());
        m_blotter = std::make_unique<BlotterController>(m_model, m_tasks,
          m_collection, std::make_shared<CancelKeyBindingsModel>(),
          std::make_shared<PositionKeyBindingsModel>(),
          Ref(m_flatten_controller), Ref(m_profile));
        m_controller = std::make_unique<AccountOrderController>(
          *m_blotter, m_tasks, m_orders, m_profile);
        m_profile.GetKeyBindings()->get_interactions_key_bindings(
          parse_ticker("RY.TSX"))->is_cancel_on_fill()->set(true);
        flush();
      }

      ~ControllerFixture() {
        m_controller.reset();
        for(auto& task : m_owned_tasks) {
          task->GetExecutor().Close();
        }
        for(auto& entry : std::as_const(*m_tasks)) {
          entry.m_task->GetExecutor().Close();
        }
        flush_pending_routines();
      }

      auto make_task() {
        auto task = std::make_shared<Task>(
          IntegerNode(0), m_model->get_account(), Ref(m_profile));
        m_owned_tasks.push_back(task);
        return task;
      }

      auto add_task() {
        auto task = make_task();
        auto entry = BlotterTaskEntry();
        entry.m_task = task;
        m_tasks->push(entry);
        return task;
      }

      auto make_order() {
        auto order = std::make_shared<ReportingOrder>(
          OrderInfo(make_limit_order_fields(m_model->get_account(),
            parse_ticker("RY.TSX"), Currencies::CAD, Side::BID, "TSX", 100,
            Money(10)), m_next_id, time_from_string("2026-11-23 15:00:00")));
        ++m_next_id;
        return order;
      }

      auto add_order(OrderStatus status) {
        auto order = make_order();
        if(status != OrderStatus::NEW) {
          order->publish(status);
        }
        add_task()->GetContext().GetOrderPublisher().push(order);
        m_orders->push(order);
        flush();
        return order;
      }

      auto add_order() {
        return add_order(OrderStatus::NEW);
      }

      auto get_cancellations() {
        auto ids = std::vector<OrderId>();
        while(auto id = m_cancellations->try_pop()) {
          ids.push_back(*id);
        }
        return ids;
      }
  };

  void wait_for(auto& model, const auto& action, const auto& condition) {
    struct Waiter {
      const std::remove_reference_t<decltype(condition)>* m_condition;
      QEventLoop m_loop;

      Waiter(decltype(condition) condition)
        : m_condition(&condition) {}

      void on_operation(
          const typename std::remove_reference_t<decltype(model)>::
            Operation&) {
        if((*m_condition)()) {
          m_loop.quit();
        }
      }
    };
    auto waiter = Waiter(condition);
    auto connection = scoped_connection(model.connect_operation_signal(
      std::bind_front(&Waiter::on_operation, &waiter)));
    action();
    if(!condition()) {
      waiter.m_loop.exec();
    }
    REQUIRE(condition());
  }

  auto get_status(const Order& order) {
    return order.get_publisher().with([] (const auto& reports) {
      return reports->back().m_status;
    });
  }

  void wait_for_status(
      const Order& order, OrderStatus status, const auto& action) {
    struct Waiter {
      OrderStatus m_status;
      QEventLoop m_loop;

      Waiter(OrderStatus status)
        : m_status(status) {}

      void on_report(const ExecutionReport& report) {
        if(report.m_status == m_status) {
          m_loop.quit();
        }
      }
    };
    auto waiter = Waiter(status);
    auto events = QtTaskQueue();
    order.get_publisher().monitor(events.get_slot<ExecutionReport>(
      std::bind_front(&Waiter::on_report, &waiter)));
    action();
    if(get_status(order) != status) {
      waiter.m_loop.exec();
    }
    REQUIRE(get_status(order) == status);
  }
}

TEST_SUITE("AccountOrderController") {
  UI_TEST_CASE("recovered_order_tasks") {
    auto fixture = Fixture();
    auto completed = fixture.submit();
    fixture.m_environment.fill(*completed, 100);
    auto live = fixture.submit();
    fixture.m_environment.set(live->get_info().m_timestamp);
    auto global =
      fixture.m_profile.GetBlotterRegistry().get_consolidated().get_model();
    wait_for(*global->get_tasks(), [] {
    },
    [&] {
      return global->get_tasks()->get_size() == 1 &&
        global->get_tasks()->get(0).m_state == Task::State::ACTIVE;
    });
    fixture.m_profile.GetKeyBindings()->get_interactions_key_bindings(
      parse_ticker("RY.TSX"))->is_cancel_on_fill()->set(true);
    auto registry = BlotterRegistry(Ref(fixture.m_profile));
    auto& controller = registry.get_consolidated();
    auto model = controller.get_model();
    wait_for(*model->get_tasks(), [] {
    },
    [&] {
      return model->get_tasks()->get_size() == 1 &&
        model->get_tasks()->get(0).m_state == Task::State::ACTIVE;
    });
    auto task = model->get_tasks()->get(0).m_task;
    auto node = dynamic_cast<const OrderWrapperTaskNode*>(&task->GetNode());
    REQUIRE(node);
    REQUIRE(node->GetOrder()->get_info().m_id == live->get_info().m_id);
    REQUIRE(get_status(*live) == OrderStatus::NEW);
    wait_for_status(*live, OrderStatus::PENDING_CANCEL, [&] {
      controller.cancel(CancelKeyBindingsModel::Operation::ALL_BIDS,
        parse_ticker("RY.TSX"), {}, boost::none);
    });
    fixture.m_environment.cancel(*live);
    wait_for(*model->get_tasks(), [] {
    },
    [&] {
      return model->get_tasks()->get(0).m_state == Task::State::CANCELED;
    });
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("task_order_ownership") {
    auto fixture = Fixture();
    auto& registry = fixture.m_profile.GetBlotterRegistry();
    auto global = registry.get_consolidated().get_model();
    auto& source = registry.create("Source", fixture.get_account());
    auto order = fixture.submit();
    SUBCASE("task_first") {
    }
    SUBCASE("account_first") {
      wait_for(*global->get_tasks(), [] {
      },
      [&] {
        return global->get_tasks()->get_size() == 1 &&
          global->get_tasks()->get(0).m_state == Task::State::ACTIVE;
      });
    }
    auto task = source.add(IntegerNode(0));
    auto client_order = fixture.m_profile.GetClients().
      get_order_execution_client().load_order(order->get_info().m_id);
    wait_for(*global->get_tasks(), [&] {
      task->GetContext().GetOrderPublisher().push(client_order);
    },
    [&] {
      return global->get_tasks()->get_size() == 1 &&
        global->get_tasks()->get(0).m_task == task;
    });
    wait_for(*global->get_orders(), [] {
    },
    [&] {
      return global->get_orders()->get_size() == 1;
    });
    flush_pending_routines();
    QApplication::sendPostedEvents();
    REQUIRE(global->get_tasks()->get_size() == 1);
    REQUIRE(global->get_tasks()->get(0).m_task == task);
    REQUIRE(get_status(*order) == OrderStatus::NEW);
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("cancel_on_fill") {
    auto fixture = Fixture();
    auto& registry = fixture.m_profile.GetBlotterRegistry();
    auto global = registry.get_consolidated().get_model();
    auto ticker = parse_ticker("RY.TSX");
    fixture.m_profile.GetKeyBindings()->get_interactions_key_bindings(ticker)->
      is_cancel_on_fill()->set(true);
    auto& other = registry.create("Other", fixture.get_account());
    registry.set_active(*other.get_model());
    auto filled = fixture.submit();
    auto target = fixture.submit();
    auto opposite = fixture.submit(ticker, Side::ASK);
    auto different = fixture.submit(parse_ticker("BMO.TSX"), Side::BID);
    wait_for(*global->get_tasks(), [] {
    },
    [&] {
      return global->get_tasks()->get_size() == 4;
    });
    wait_for_status(*filled, OrderStatus::PARTIALLY_FILLED, [&] {
      fixture.m_environment.fill(*filled, 40);
    });
    flush_pending_routines();
    QApplication::sendPostedEvents();
    REQUIRE(get_status(*target) == OrderStatus::NEW);
    wait_for_status(*target, OrderStatus::PENDING_CANCEL, [&] {
      fixture.m_environment.fill(*filled, 60);
    });
    REQUIRE(get_status(*opposite) == OrderStatus::NEW);
    REQUIRE(get_status(*different) == OrderStatus::NEW);
    REQUIRE(!registry.get_consolidated().is_open()->get());
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("cancel_on_fill_disabled") {
    auto fixture = Fixture();
    auto& registry = fixture.m_profile.GetBlotterRegistry();
    auto model = registry.get_consolidated().get_model();
    auto filled = fixture.submit();
    auto target = fixture.submit();
    wait_for(*model->get_tasks(), [] {
    },
    [&] {
      return model->get_tasks()->get_size() == 2;
    });
    wait_for(*model->get_tasks(), [&] {
      fixture.m_environment.fill(*filled, 100);
    },
    [&] {
      return std::ranges::any_of(std::as_const(*model->get_tasks()),
        [] (const auto& entry) {
          return entry.m_state == Task::State::COMPLETE;
        });
    });
    REQUIRE(get_status(*target) == OrderStatus::NEW);
  }

  UI_TEST_CASE("cancel_on_fill_before_delivery") {
    auto fixture = Fixture();
    auto& registry = fixture.m_profile.GetBlotterRegistry();
    auto model = registry.get_consolidated().get_model();
    fixture.m_profile.GetKeyBindings()->get_interactions_key_bindings(
      parse_ticker("RY.TSX"))->is_cancel_on_fill()->set(true);
    auto target = fixture.submit();
    wait_for(*model->get_tasks(), [] {
    },
    [&] {
      return model->get_tasks()->get_size() == 1;
    });
    wait_for_status(*target, OrderStatus::PENDING_CANCEL, [&] {
      auto filled = fixture.submit();
      fixture.m_environment.fill(*filled, 100);
    });
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("cancel_on_fill_account_isolation") {
    auto is_other_account = false;
    SUBCASE("logged_in_account") {
    }
    SUBCASE("other_account") {
      is_other_account = true;
    }
    auto fixture = ControllerFixture(is_other_account);
    auto filled = fixture.add_order();
    auto target = fixture.add_order();
    filled->publish(OrderStatus::FILLED);
    flush();
    auto cancellations = fixture.get_cancellations();
    if(is_other_account) {
      REQUIRE(cancellations.empty());
    } else {
      REQUIRE(cancellations == std::vector{target->get_info().m_id});
    }
  }

  UI_TEST_CASE("task_subscription_cleanup") {
    auto fixture = ControllerFixture();
    auto original = fixture.add_task();
    auto retained = fixture.add_task();
    flush();
    SUBCASE("remove") {
      fixture.m_tasks->remove(0);
    }
    SUBCASE("replace") {
      retained = fixture.make_task();
      auto entry = BlotterTaskEntry();
      entry.m_task = retained;
      fixture.m_tasks->set(0, entry);
    }
    SUBCASE("move_then_remove") {
      fixture.m_tasks->move(0, 1);
      auto order = fixture.make_order();
      fixture.m_orders->push(order);
      flush();
      REQUIRE(fixture.m_tasks->get_size() == 3);
      original->GetContext().GetOrderPublisher().push(order);
      flush();
      REQUIRE(fixture.m_tasks->get_size() == 2);
      fixture.m_tasks->remove(1);
    }
    auto size = fixture.m_tasks->get_size();
    original->GetContext().GetOrderPublisher().push(fixture.make_order());
    flush();
    REQUIRE(fixture.m_tasks->get_size() == size);
    auto order = fixture.make_order();
    fixture.m_orders->push(order);
    flush();
    REQUIRE(fixture.m_tasks->get_size() == size + 1);
    retained->GetContext().GetOrderPublisher().push(order);
    flush();
    REQUIRE(fixture.m_tasks->get_size() == size);
  }

  UI_TEST_CASE("cancel_on_fill_deduplication") {
    auto fixture = ControllerFixture();
    auto filled = fixture.add_order();
    auto target = fixture.add_order();
    filled->publish(OrderStatus::FILLED);
    flush();
    REQUIRE(
      fixture.get_cancellations() == std::vector{target->get_info().m_id});
    auto next = fixture.add_order();
    filled->replay();
    flush();
    REQUIRE(fixture.get_cancellations().empty());
    next->publish(OrderStatus::FILLED);
    flush();
    REQUIRE(fixture.get_cancellations().empty());
  }

  UI_TEST_CASE("cancel_on_fill_ineligible_orders") {
    auto fixture = ControllerFixture();
    for(auto status : {OrderStatus::FILLED, OrderStatus::CANCELED,
        OrderStatus::REJECTED, OrderStatus::EXPIRED,
        OrderStatus::PENDING_CANCEL}) {
      fixture.add_order(status);
    }
    auto filled = fixture.add_order();
    auto target = fixture.add_order();
    REQUIRE(fixture.get_cancellations().empty());
    filled->publish(OrderStatus::FILLED);
    flush();
    REQUIRE(
      fixture.get_cancellations() == std::vector{target->get_info().m_id});
  }

  UI_TEST_CASE("controller_destruction") {
    auto fixture = ControllerFixture();
    auto filled = fixture.add_order();
    fixture.add_order();
    auto task = fixture.add_task();
    flush();
    SUBCASE("pending_callbacks") {
      filled->publish(OrderStatus::FILLED);
      task->GetContext().GetOrderPublisher().push(fixture.make_order());
      fixture.m_orders->push(fixture.make_order());
    }
    SUBCASE("idle") {
    }
    fixture.m_controller.reset();
    flush();
    REQUIRE(fixture.m_tasks->get_size() == 3);
    REQUIRE(fixture.get_cancellations().empty());
    filled->publish(OrderStatus::FILLED);
    fixture.m_orders->push(fixture.make_order());
    task->GetContext().GetOrderPublisher().push(fixture.make_order());
    auto added = fixture.add_task();
    added->GetContext().GetOrderPublisher().push(fixture.make_order());
    flush();
    REQUIRE(fixture.m_tasks->get_size() == 4);
    REQUIRE(fixture.get_cancellations().empty());
  }
}
