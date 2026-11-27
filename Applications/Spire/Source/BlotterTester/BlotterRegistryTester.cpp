#include <QEvent>
#include <QEventLoop>
#include <QPointer>
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterRegistry.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderWrapperTaskNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::Tests;
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
        make_limit_order_fields(get_account(), ticker, Currencies::CAD, side,
        "TSX", 100, Money(10)));
      auto order = m_submissions->pop();
      m_environment.accept(*order);
      return order;
    }

    auto submit() {
      return submit(parse_ticker("RY.TSX"), Side::BID);
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

  auto get_quantity(const BlotterModel& model) {
    auto quantity = Quantity();
    for(auto& entry : std::as_const(*model.get_portfolio())) {
      quantity += entry.m_inventory.m_position.m_quantity;
    }
    return quantity;
  }
}

TEST_SUITE("BlotterRegistry") {
  UI_TEST_CASE("initial_blotter") {
    auto fixture = Fixture();
    auto registry = BlotterRegistry(Ref(fixture.m_profile));
    auto& controller = registry.get_consolidated();
    auto model = controller.get_model();
    REQUIRE(registry.get_blotters()->get_size() == 1);
    REQUIRE(registry.get_available()->get_size() == 1);
    REQUIRE(registry.get_pinned()->get_size() == 0);
    REQUIRE(model->is_consolidated());
    REQUIRE(model->is_pinned()->get());
    REQUIRE(model->get_account() == fixture.get_account());
    REQUIRE(model->get_name()->get() == "Global");
    REQUIRE(registry.get_active()->get() == model);
    REQUIRE(model->is_active()->get());
    REQUIRE_FALSE(controller.is_open()->get());
    REQUIRE(&registry.get(*model) == &controller);
    REQUIRE(&registry.get_consolidated(fixture.get_account()) == &controller);
    REQUIRE(model->is_pinned()->set(false) == QValidator::Invalid);
    controller.open();
    controller.close();
    REQUIRE(registry.get_blotters()->get_size() == 1);
    REQUIRE(registry.get_active()->get() == model);
  }

  UI_TEST_CASE("create_and_activate") {
    auto fixture = Fixture();
    auto registry = BlotterRegistry(Ref(fixture.m_profile));
    auto global = registry.get_active()->get();
    auto first = registry.create(" Orders ", fixture.get_account()).get_model();
    auto second = registry.create("Orders", fixture.get_account()).get_model();
    REQUIRE(first->get_name()->get() == "Orders");
    REQUIRE(second->get_name()->get() == "Orders (2)");
    REQUIRE_FALSE(first->is_consolidated());
    REQUIRE_FALSE(first->is_pinned()->get());
    REQUIRE(registry.get_available()->get_size() == 1);
    REQUIRE(registry.get_blotters()->get_size() == 3);
    registry.set_active(*first);
    REQUIRE(registry.get_active()->get() == first);
    REQUIRE(first->is_active()->get());
    REQUIRE_FALSE(global->is_active()->get());
    second->is_active()->set(true);
    REQUIRE(registry.get_active()->get() == second);
    REQUIRE_FALSE(first->is_active()->get());
    second->is_active()->set(false);
    REQUIRE(second->is_active()->get());
    REQUIRE(registry.get_active()->get() == second);
    second->get_name()->set("Changed");
    REQUIRE(registry.create("Orders", fixture.get_account()).get_model()->
      get_name()->get() == "Orders (2)");
  }

  UI_TEST_CASE("close_unpinned") {
    auto fixture = Fixture();
    auto registry = BlotterRegistry(Ref(fixture.m_profile));
    auto global = registry.get_active()->get();
    auto& controller = registry.create("Temporary", fixture.get_account());
    auto model = std::weak_ptr(controller.get_model());
    auto window = QPointer(&controller.open());
    registry.set_active(*controller.get_model());
    REQUIRE(registry.get_available()->get_size() == 2);
    REQUIRE(window->close());
    REQUIRE(registry.get_blotters()->get_size() == 1);
    REQUIRE(registry.get_available()->get_size() == 1);
    REQUIRE(registry.get_active()->get() == global);
    REQUIRE(global->is_active()->get());
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    REQUIRE(window.isNull());
    REQUIRE(model.expired());
  }

  UI_TEST_CASE("close_pinned") {
    auto fixture = Fixture();
    auto registry = BlotterRegistry(Ref(fixture.m_profile));
    auto& controller = registry.create("Pinned", fixture.get_account());
    auto model = controller.get_model();
    model->is_pinned()->set(true);
    registry.set_active(*model);
    REQUIRE(registry.get_pinned()->get_size() == 1);
    REQUIRE(registry.get_available()->get_size() == 2);
    controller.open();
    controller.close();
    REQUIRE_FALSE(controller.is_open()->get());
    REQUIRE(registry.get_blotters()->get_size() == 2);
    REQUIRE(registry.get_available()->get_size() == 2);
    REQUIRE(registry.get_active()->get() == model);
    REQUIRE(&registry.get(*model) == &controller);
    SUBCASE("reopen") {
      REQUIRE(controller.open().get_model() == model);
      model->is_pinned()->set(false);
      REQUIRE(registry.get_pinned()->get_size() == 0);
      REQUIRE(registry.get_available()->get_size() == 2);
      controller.close();
    }
    SUBCASE("unpin_closed") {
      model->is_pinned()->set(false);
    }
    REQUIRE(registry.get_blotters()->get_size() == 1);
    REQUIRE(registry.get_available()->get_size() == 1);
    REQUIRE(registry.get_pinned()->get_size() == 0);
    REQUIRE(registry.get_active()->get()->is_consolidated());
  }

  UI_TEST_CASE("close_all") {
    auto fixture = Fixture();
    auto registry = BlotterRegistry(Ref(fixture.m_profile));
    auto& pinned = registry.create("Pinned", fixture.get_account());
    pinned.get_model()->is_pinned()->set(true);
    pinned.open();
    registry.get_consolidated().open();
    registry.create("First", fixture.get_account()).open();
    auto& last = registry.create("Last", fixture.get_account());
    last.open();
    registry.set_active(*last.get_model());
    registry.close();
    REQUIRE(registry.get_blotters()->get_size() == 2);
    REQUIRE(registry.get_available()->get_size() == 2);
    REQUIRE(registry.get_pinned()->get_size() == 1);
    REQUIRE(registry.get_active()->get()->is_consolidated());
    REQUIRE_FALSE(pinned.is_open()->get());
    REQUIRE_FALSE(registry.get_consolidated().is_open()->get());
    registry.close();
    REQUIRE(registry.get_blotters()->get_size() == 2);
  }

  UI_TEST_CASE("account_consolidation") {
    auto fixture = Fixture();
    auto account = fixture.m_profile.GetClients().get_service_locator_client().
      make_account("other", "", DirectoryEntry::STAR_DIRECTORY);
    auto registry = BlotterRegistry(Ref(fixture.m_profile));
    registry.create("Other", account);
    auto& controller = registry.get_consolidated(account);
    REQUIRE(registry.get_blotters()->get_size() == 3);
    REQUIRE(controller.get_model()->get_account() == account);
    REQUIRE(controller.get_model()->is_consolidated());
    REQUIRE_FALSE(controller.get_model()->is_pinned()->get());
    REQUIRE(&registry.get_consolidated(account) == &controller);
    REQUIRE(&registry.get_consolidated() != &controller);
    controller.open();
    registry.set_active(*controller.get_model());
    controller.close();
    REQUIRE(registry.get_blotters()->get_size() == 2);
    REQUIRE(registry.get_active()->get()->get_account() ==
      fixture.get_account());
    REQUIRE(registry.get_consolidated(account).get_model()->is_consolidated());
    REQUIRE(registry.get_blotters()->get_size() == 3);
  }

  UI_TEST_CASE("default_order_quantity") {
    auto fixture = Fixture();
    auto& registry = fixture.m_profile.GetBlotterRegistry();
    auto ticker = parse_ticker("RY.TSX");
    auto interactions = fixture.m_profile.GetKeyBindings()->
      get_interactions_key_bindings(ticker);
    interactions->get_default_quantity()->set(100);
    auto quantity = [&] (Side side) {
      return get_default_order_quantity(fixture.m_profile, ticker, side);
    };
    REQUIRE(quantity(Side::BID) == 100);
    REQUIRE(quantity(Side::ASK) == 100);
    auto& first = registry.create("First", fixture.get_account());
    auto& second = registry.create("Second", fixture.get_account());
    auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
      make_limit_order_fields(ticker, Currencies::CAD, Side::BID, "TSX", 300,
        Money(10)), 1, time_from_string("2026-11-23 15:00:00")));
    accept(*order);
    fill(*order, 40);
    auto task = first.add(IntegerNode(0));
    wait_for(*first.get_model()->get_portfolio(), [&] {
      task->GetContext().GetOrderPublisher().push(order);
    }, [&] {
      return get_quantity(*first.get_model()) == 40;
    });
    registry.set_active(*first.get_model());
    REQUIRE(quantity(Side::BID) == 60);
    REQUIRE(quantity(Side::ASK) == 40);
    REQUIRE(get_default_order_quantity(fixture.m_profile,
      parse_ticker("BMO.TSX"), Side::ASK) == 100);
    wait_for(*first.get_model()->get_portfolio(), [&] {
      fill(*order, 100);
    }, [&] {
      return get_quantity(*first.get_model()) == 140;
    });
    REQUIRE(quantity(Side::BID) == 60);
    REQUIRE(quantity(Side::ASK) == 100);
    registry.set_active(*second.get_model());
    REQUIRE(quantity(Side::BID) == 100);
    REQUIRE(quantity(Side::ASK) == 100);
    auto short_order = std::make_shared<PrimitiveOrder>(OrderInfo(
      make_limit_order_fields(ticker, Currencies::CAD, Side::ASK, "TSX", 100,
        Money(10)), 2, time_from_string("2026-11-23 15:00:00")));
    accept(*short_order);
    fill(*short_order, 25);
    auto other_task = second.add(IntegerNode(0));
    wait_for(*second.get_model()->get_portfolio(), [&] {
      other_task->GetContext().GetOrderPublisher().push(short_order);
    }, [&] {
      return get_quantity(*second.get_model()) == -25;
    });
    REQUIRE(quantity(Side::BID) == 25);
    REQUIRE(quantity(Side::ASK) == 75);
    registry.set_active(*first.get_model());
    REQUIRE(quantity(Side::BID) == 60);
    REQUIRE(quantity(Side::ASK) == 100);
    interactions->get_default_quantity()->set(200);
    REQUIRE(quantity(Side::BID) == 60);
    REQUIRE(quantity(Side::ASK) == 140);
    interactions->get_default_quantity()->set(0);
    REQUIRE(quantity(Side::BID) == 0);
    REQUIRE(quantity(Side::ASK) == 0);
    flush_pending_routines();
  }

  UI_TEST_CASE("shared_flatten_controller") {
    auto fixture = Fixture();
    auto& registry = fixture.m_profile.GetBlotterRegistry();
    auto& consolidated = registry.get_consolidated();
    auto& source = registry.create("Source", fixture.get_account());
    auto ticker = parse_ticker("RY.TSX");
    auto initial = fixture.submit();
    fixture.m_environment.fill(*initial, 100);
    auto order =
      fixture.m_profile.GetClients().get_order_execution_client().load_order(
        initial->get_info().m_id);
    auto task = source.add(IntegerNode(0));
    wait_for(*source.get_model()->get_orders(), [&] {
      task->GetContext().GetOrderPublisher().push(order);
    }, [&] {
      return source.get_model()->get_orders()->get_size() == 1;
    });
    source.flatten(ticker);
    auto flatten = fixture.m_submissions->pop();
    auto pending = source.get_model()->get_tasks()->get(1).m_task;
    consolidated.flatten(ticker);
    source.flatten(ticker);
    pending->GetExecutor().Close();
    REQUIRE(flatten->get_info().m_fields.m_side == Side::ASK);
    REQUIRE(flatten->get_info().m_fields.m_quantity == 100);
    REQUIRE(source.get_model()->get_tasks()->get_size() == 2);
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("account_orders_and_positions") {
    auto fixture = Fixture();
    auto completed = fixture.submit();
    fixture.m_environment.fill(*completed, 100);
    flush_pending_routines();
    auto snapshot = fixture.m_profile.GetClients().get_risk_client().
      load_inventory_snapshot(fixture.get_account());
    REQUIRE(snapshot.m_inventories.size() == 1);
    REQUIRE(snapshot.m_inventories.front().m_position.m_quantity == 100);
    auto initial = fixture.submit();
    fixture.m_environment.set(initial->get_info().m_timestamp);
    fixture.m_environment.fill(*initial, 40);
    flush_pending_routines();
    auto registry = BlotterRegistry(Ref(fixture.m_profile));
    auto model = registry.get_consolidated().get_model();
    wait_for(*model->get_orders(), [] {
    }, [&] {
      return model->get_orders()->get_size() == 2;
    });
    wait_for(*model->get_portfolio(), [] {
    }, [&] {
      return get_quantity(*model) == 140;
    });
    wait_for(*model->get_tasks(), [] {
    },
    [&] {
      return model->get_tasks()->get_size() == 1 &&
        model->get_tasks()->get(0).m_state == Task::State::ACTIVE;
    });
    auto task = model->get_tasks()->get(0).m_task;
    auto node = dynamic_cast<const OrderWrapperTaskNode*>(&task->GetNode());
    REQUIRE(node);
    REQUIRE(node->GetOrder()->get_info().m_id == initial->get_info().m_id);
    wait_for(*model->get_portfolio(), [&] {
      fixture.m_environment.fill(*initial, 60);
    }, [&] {
      return get_quantity(*model) == 200;
    });
    auto next = std::shared_ptr<Order>();
    wait_for(*model->get_orders(), [&] {
      next = fixture.submit();
    }, [&] {
      return model->get_orders()->get_size() == 3;
    });
    wait_for(*model->get_portfolio(), [&] {
      fixture.m_environment.fill(*next, 25);
    }, [&] {
      return get_quantity(*model) == 225;
    });
  }
}
