#include <QEventLoop>
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/BookView/ServiceBookViewModel.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ProxyListModel.hpp"
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
    TestClients m_clients;
    std::shared_ptr<OrdersModel> m_orders;

    Fixture()
      : m_clients(Ref(m_environment)),
        m_orders(std::make_shared<ArrayListModel<std::shared_ptr<Order>>>()) {}

    ~Fixture() {
      flush_pending_routines();
    }

    auto make_model() {
      return std::make_unique<ServiceBookViewModel>(parse_ticker("RY.TSX"),
        m_clients.get_market_data_client(), m_orders);
    }
  };

  auto make_order(OrderId id, const Ticker& ticker, Side side) {
    return std::make_shared<PrimitiveOrder>(OrderInfo(
      make_limit_order_fields(ticker, Currencies::CAD, side, "TSX", 100,
        Money(10)), id, time_from_string("2026-11-23 15:00:00")));
  }

  void wait_for(auto& model, const auto& action, const auto& condition) {
    auto loop = QEventLoop();
    auto connection = scoped_connection(
      model.connect_operation_signal([&] (const auto&) {
        if(condition()) {
          loop.quit();
        }
      }));
    action();
    if(!condition()) {
      loop.exec();
    }
    REQUIRE(condition());
  }
}

TEST_SUITE("ServiceBookViewModel") {
  UI_TEST_CASE("order_snapshot") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("RY.TSX");
    auto bid = make_order(1, ticker, Side::BID);
    accept(*bid);
    fill(*bid, 30);
    auto ask = make_order(2, ticker, Side::ASK);
    accept(*ask);
    auto terminal = make_order(3, ticker, Side::BID);
    accept(*terminal);
    fill(*terminal, 100);
    auto other = make_order(4, parse_ticker("BMO.TSX"), Side::BID);
    fixture.m_orders->push(bid);
    fixture.m_orders->push(ask);
    fixture.m_orders->push(terminal);
    fixture.m_orders->push(other);
    auto model = fixture.make_model();
    auto bids = model->get_bid_orders();
    auto asks = model->get_ask_orders();
    REQUIRE(bids->get_size() == 1);
    REQUIRE(bids->get(0).m_id == 1);
    REQUIRE(bids->get(0).m_size == 70);
    REQUIRE(bids->get(0).m_status == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(asks->get_size() == 1);
    REQUIRE(asks->get(0).m_id == 2);
    REQUIRE(asks->get(0).m_status == OrderStatus::NEW);
    QApplication::processEvents();
    REQUIRE(bids->get(0).m_size == 70);
  }

  UI_TEST_CASE("order_updates") {
    auto fixture = Fixture();
    auto model = fixture.make_model();
    auto bids = model->get_bid_orders();
    auto order = make_order(1, parse_ticker("RY.TSX"), Side::BID);
    fixture.m_orders->push(order);
    REQUIRE(bids->get_size() == 1);
    REQUIRE(bids->get(0).m_status == OrderStatus::PENDING_NEW);
    wait_for(*bids, [&] {
      accept(*order);
    }, [&] {
      return bids->get(0).m_status == OrderStatus::NEW;
    });
    wait_for(*bids, [&] {
      fill(*order, 40);
    }, [&] {
      return bids->get(0).m_size == 60;
    });
    wait_for(*bids, [&] {
      set_order_status(*order, OrderStatus::PENDING_CANCEL);
      cancel(*order);
    }, [&] {
      return bids->get_size() == 0;
    });
  }

  UI_TEST_CASE("order_list_changes") {
    auto fixture = Fixture();
    auto model = fixture.make_model();
    auto ticker = parse_ticker("RY.TSX");
    auto bid = make_order(1, ticker, Side::BID);
    accept(*bid);
    fixture.m_orders->push(bid);
    fixture.m_orders->push(
      make_order(2, parse_ticker("BMO.TSX"), Side::BID));
    auto ask = make_order(3, ticker, Side::ASK);
    fixture.m_orders->insert(std::shared_ptr<Order>(ask), 0);
    fixture.m_orders->move(0, 2);
    fixture.m_orders->remove(1);
    REQUIRE(model->get_bid_orders()->get_size() == 1);
    REQUIRE(model->get_ask_orders()->get_size() == 1);
    fixture.m_orders->remove(1);
    REQUIRE(model->get_ask_orders()->get_size() == 0);
    auto replacement = make_order(4, ticker, Side::ASK);
    fill(*bid, 30);
    fixture.m_orders->set(0, std::shared_ptr<Order>(replacement));
    QApplication::processEvents();
    REQUIRE(model->get_bid_orders()->get_size() == 0);
    REQUIRE(model->get_ask_orders()->get_size() == 1);
    REQUIRE(model->get_ask_orders()->get(0).m_id == 4);
    REQUIRE(model->get_ask_orders()->get(0).m_size == 100);
  }

  UI_TEST_CASE("order_source_switch") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("RY.TSX");
    auto order = make_order(1, ticker, Side::BID);
    accept(*order);
    fixture.m_orders->push(order);
    auto source = std::make_shared<ProxyListModel<std::shared_ptr<Order>>>(
      fixture.m_orders);
    auto model = ServiceBookViewModel(
      ticker, fixture.m_clients.get_market_data_client(), source);
    auto bids = model.get_bid_orders();
    auto next = std::make_shared<ArrayListModel<std::shared_ptr<Order>>>();
    next->push(order);
    next->push(make_order(2, ticker, Side::ASK));
    fill(*order, 30);
    source->set_source(next);
    REQUIRE(bids->get_size() == 1);
    REQUIRE(bids->get(0).m_size == 70);
    QApplication::processEvents();
    REQUIRE(bids->get(0).m_size == 70);
    REQUIRE(model.get_ask_orders()->get_size() == 1);
    fixture.m_orders->push(make_order(3, ticker, Side::BID));
    REQUIRE(bids->get_size() == 1);
    wait_for(*bids, [&] {
      fill(*order, 20);
    }, [&] {
      return bids->get(0).m_size == 50;
    });
  }

  UI_TEST_CASE("active_blotter") {
    auto environment = TestEnvironment();
    auto profile = UserProfile("", false, false, {}, {},
      get_default_additional_tag_database(), {}, {}, {},
      Clients(std::in_place_type<TestClients>, Ref(environment)));
    auto& registry = profile.GetBlotterRegistry();
    auto account =
      profile.GetClients().get_service_locator_client().get_account();
    auto& first = registry.create("First", account);
    auto& second = registry.create("Second", account);
    registry.set_active(*first.get_model());
    auto ticker = parse_ticker("RY.TSX");
    auto model = profile.GetBookViewModelBuilder()(ticker);
    auto bids = model->get_bid_orders();
    auto asks = model->get_ask_orders();
    auto task = first.add(IntegerNode(0));
    auto order = make_order(1, ticker, Side::BID);
    accept(*order);
    fill(*order, 25);
    wait_for(*bids, [&] {
      task->GetContext().GetOrderPublisher().push(order);
    }, [&] {
      return bids->get_size() == 1;
    });
    REQUIRE(bids->get(0).m_size == 75);
    registry.set_active(*second.get_model());
    REQUIRE(bids->get_size() == 0);
    auto other_task = second.add(IntegerNode(0));
    wait_for(*asks, [&] {
      other_task->GetContext().GetOrderPublisher().push(
        make_order(2, ticker, Side::ASK));
    }, [&] {
      return asks->get_size() == 1;
    });
    fill(*order, 10);
    registry.set_active(*first.get_model());
    REQUIRE(asks->get_size() == 0);
    REQUIRE(bids->get_size() == 1);
    REQUIRE(bids->get(0).m_size == 65);
    QApplication::processEvents();
    REQUIRE(bids->get(0).m_size == 65);
    REQUIRE(profile.GetBookViewModelBuilder()(ticker) == model);
    flush_pending_routines();
  }
}
