#include <deque>
#include <thread>
#include <Beam/Queues/Queue.hpp>
#include <QEventLoop>
#include <QTimer>
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Spire/Blotter/ControllerPortfolioModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Tests;
using namespace Spire;

namespace {
  struct Fixture {
    ServiceLocatorTestEnvironment m_service_locator;
    AdministrationServiceTestEnvironment m_administration;
    MarketDataServiceTestEnvironment m_market_data;
    std::shared_ptr<Queue<std::shared_ptr<Order>>> m_orders;

    Fixture()
      : m_administration(
          make_administration_service_test_environment(m_service_locator)),
        m_market_data(make_market_data_service_test_environment(
          m_service_locator, m_administration)),
        m_orders(std::make_shared<Queue<std::shared_ptr<Order>>>()) {}

    void publish(const Ticker& ticker, Money bid, Money ask, ptime timestamp) {
      m_market_data.get_feed_client().publish(TickerBboQuote(
        BboQuote(make_bid(bid, 100), make_ask(ask, 100), timestamp), ticker));
    }

    auto make_controller(const std::vector<Inventory>& inventories) {
      for(auto& inventory : inventories) {
        publish(inventory.m_position.m_ticker, Money(12), Money(13),
          time_from_string("2026-11-21 10:00:00"));
      }
      return std::make_shared<ControllerPortfolioModel::Controller>(
        Portfolio(TrueAverageBookkeeper(inventories)),
        m_market_data.get_registry_client(), m_orders);
    }
  };

  int find_index(const PortfolioModel& model, const Ticker& ticker) {
    for(auto i = 0; i != model.get_size(); ++i) {
      if(model.get(i).m_inventory.m_position.m_ticker == ticker) {
        return i;
      }
    }
    throw std::runtime_error("Portfolio entry not found.");
  }

  auto make_order(OrderId id, const Ticker& ticker, Side side,
      Quantity quantity, Money price) {
    return std::make_shared<PrimitiveOrder>(OrderInfo(
      make_limit_order_fields(ticker, CAD, side, "TSX", quantity, price), id,
      time_from_string("2026-11-21 10:00:00")));
  }

  void wait_for(
      PortfolioModel& model, const auto& action, const auto& condition) {
    auto loop = QEventLoop();
    auto connection = boost::signals2::scoped_connection(
      model.connect_operation_signal([&] (const auto&) {
        if(condition()) {
          loop.quit();
        }
      }));
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    action();
    if(!condition()) {
      loop.exec();
    }
    REQUIRE(condition());
  }
}

TEST_SUITE("ControllerPortfolioModel") {
  UI_TEST_CASE("empty") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller({});
    auto model = ControllerPortfolioModel(controller);
    REQUIRE(model.get_size() == 0);
    REQUIRE_THROWS_AS(model.get(0), std::out_of_range);
  }

  UI_TEST_CASE("initial_snapshot") {
    auto fixture = Fixture();
    auto inventories = std::vector{
      Inventory(Position(parse_ticker("RY.TSX"), CAD, 100, Money(1000)),
        Money(3), Money(1), 150, 2),
      Inventory(Position(parse_ticker("TD.TSX"), CAD, -50, Money(-500)),
        Money(4), Money(2), 80, 3),
      Inventory(Position(parse_ticker("BMO.TSX"), CAD, 0, Money::ZERO),
        Money(20), Money(3), 70, 4)};
    auto controller = fixture.make_controller(inventories);
    auto model = ControllerPortfolioModel(controller);
    REQUIRE(model.get_size() == 3);
    for(auto& inventory : inventories) {
      auto index = find_index(model, inventory.m_position.m_ticker);
      auto& entry = model.get(index);
      REQUIRE(entry.m_inventory == inventory);
      REQUIRE(entry.m_currency_inventory.m_position.m_currency == CAD);
      REQUIRE(entry.m_currency_inventory.m_position.m_quantity == 150);
      REQUIRE(entry.m_currency_inventory.m_position.m_cost_basis ==
        Money(1500));
      REQUIRE(entry.m_currency_inventory.m_fees == Money(6));
      REQUIRE(entry.m_currency_unrealized == Money(50));
    }
    REQUIRE(model.get(find_index(model, parse_ticker("RY.TSX"))).
      m_unrealized == Money(200));
    REQUIRE(model.get(find_index(model, parse_ticker("TD.TSX"))).
      m_unrealized == Money(-150));
    REQUIRE(model.get(find_index(model, parse_ticker("BMO.TSX"))).
      m_unrealized == Money::ZERO);
    auto operations = std::deque<PortfolioModel::Operation>();
    model.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    QApplication::processEvents();
    REQUIRE(operations.empty());
  }

  UI_TEST_CASE("portfolio_updates") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("RY.TSX");
    auto initial = Inventory(Position(ticker, CAD, 100, Money(1000)),
      Money::ZERO, Money::ZERO, 100, 1);
    auto controller = fixture.make_controller({initial});
    auto model = ControllerPortfolioModel(controller);
    auto operations = std::deque<PortfolioModel::Operation>();
    auto thread = std::this_thread::get_id();
    model.connect_operation_signal([&] (const auto& operation) {
      REQUIRE(std::this_thread::get_id() == thread);
      operations.push_back(operation);
    });
    auto order = make_order(1, ticker, Side::ASK, 100, Money(11));
    wait_for(model, [&] {
      fixture.m_orders->push(order);
      accept(*order);
      fill(*order, Money(11), 40);
    }, [&] {
      return model.get(0).m_inventory.m_position.m_quantity == 60;
    });
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_inventory.m_position.m_cost_basis == Money(600));
    REQUIRE(model.get(0).m_inventory.m_gross_profit_and_loss == Money(40));
    REQUIRE(model.get(0).m_unrealized == Money(120));
    REQUIRE(operations.size() == 1);
    auto& update = std::get<PortfolioModel::UpdateOperation>(operations[0]);
    REQUIRE(update.m_index == 0);
    REQUIRE(update.get_previous().m_inventory == initial);
    REQUIRE(update.get_value().m_inventory == model.get(0).m_inventory);
    operations.clear();
    auto inventory = model.get(0).m_inventory;
    wait_for(model, [&] {
      fixture.publish(ticker, Money(13), Money(14),
        time_from_string("2026-11-21 10:01:00"));
    }, [&] {
      return model.get(0).m_unrealized == Money(180);
    });
    REQUIRE(model.get(0).m_inventory == inventory);
    REQUIRE(model.get(0).m_currency_unrealized == Money(180));
    REQUIRE(operations.size() == 1);
    REQUIRE(std::get<PortfolioModel::UpdateOperation>(operations[0]).m_index ==
      0);
    operations.clear();
    model.start_transaction();
    wait_for(model, [&] {
      fill(*order, Money(11), 60);
    }, [&] {
      return model.get(0).m_inventory.m_position.m_quantity == 0;
    });
    model.end_transaction();
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_inventory.m_position.m_cost_basis == Money::ZERO);
    REQUIRE(model.get(0).m_inventory.m_gross_profit_and_loss == Money(100));
    REQUIRE(model.get(0).m_unrealized == Money::ZERO);
    REQUIRE(operations.size() == 3);
    REQUIRE(std::holds_alternative<PortfolioModel::StartTransaction>(
      operations.front()));
    REQUIRE(std::holds_alternative<PortfolioModel::UpdateOperation>(
      operations[1]));
    REQUIRE(std::holds_alternative<PortfolioModel::EndTransaction>(
      operations.back()));
    operations.clear();
    auto second = parse_ticker("TD.TSX");
    auto opening = make_order(2, second, Side::BID, 25, Money(10));
    fixture.publish(second, Money(12), Money(13),
      time_from_string("2026-11-21 10:01:00"));
    wait_for(model, [&] {
      fixture.m_orders->push(opening);
      accept(*opening);
      fill(*opening, Money(10), 25);
    }, [&] {
      return model.get_size() == 2;
    });
    REQUIRE(model.get(0).m_inventory.m_position.m_ticker == ticker);
    REQUIRE(model.get(1).m_inventory.m_position.m_ticker == second);
    REQUIRE(model.get(1).m_inventory.m_position.m_quantity == 25);
    REQUIRE(model.get(1).m_currency_inventory.m_gross_profit_and_loss ==
      Money(100));
    require_list_transaction<PortfolioUpdateEntry>(
      operations, {PortfolioModel::AddOperation(1)});
  }

  UI_TEST_CASE("order_list") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("RY.TSX");
    fixture.publish(ticker, Money(12), Money(13),
      time_from_string("2026-11-21 10:00:00"));
    auto orders = std::make_shared<ArrayListModel<std::shared_ptr<Order>>>();
    auto first = make_order(1, ticker, Side::BID, 100, Money(10));
    accept(*first);
    fill(*first, 40);
    orders->push(first);
    orders->push(first);
    auto model = ControllerPortfolioModel(
      orders, fixture.m_market_data.get_registry_client());
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_inventory.m_position.m_quantity == 40);
    REQUIRE(model.get(0).m_inventory.m_position.m_cost_basis == Money(400));
    wait_for(model, [&] {
      fill(*first, 20);
    }, [&] {
      return model.get(0).m_inventory.m_position.m_quantity == 60;
    });
    orders->remove(0);
    REQUIRE(model.get(0).m_inventory.m_position.m_quantity == 60);
    auto second = make_order(2, ticker, Side::ASK, 20, Money(11));
    wait_for(model, [&] {
      orders->push(second);
      accept(*second);
      fill(*second, 20);
    }, [&] {
      return model.get(0).m_inventory.m_position.m_quantity == 40;
    });
    REQUIRE(model.get(0).m_inventory.m_gross_profit_and_loss == Money(20));
    wait_for(model, [&] {
      fixture.publish(ticker, Money(14), Money(15),
        time_from_string("2026-11-21 10:01:00"));
    }, [&] {
      return model.get(0).m_unrealized == Money(160);
    });
    auto operations = std::deque<PortfolioModel::Operation>();
    model.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    orders->move(0, 1);
    orders->set(0, second);
    REQUIRE(operations.empty());
    fill(*first, 40);
    orders->transact([&] {
      orders->remove(1);
      orders->remove(0);
    });
    REQUIRE(model.get_size() == 0);
    require_list_transaction<PortfolioUpdateEntry>(operations,
      {PortfolioModel::PreRemoveOperation(0),
       PortfolioModel::RemoveOperation(0)});
    QApplication::processEvents();
    REQUIRE(model.get_size() == 0);
    auto third = make_order(3, ticker, Side::BID, 10, Money(12));
    wait_for(model, [&] {
      orders->push(third);
      accept(*third);
      fill(*third, 10);
    }, [&] {
      return model.get_size() == 1 &&
        model.get(0).m_inventory.m_position.m_quantity == 10;
    });
    REQUIRE(model.get(0).m_inventory.m_position.m_cost_basis == Money(120));
  }

  UI_TEST_CASE("historical_orders") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("RY.TSX");
    fixture.publish(ticker, Money(15), Money(16),
      time_from_string("2026-11-21 10:00:00"));
    auto opening = make_order(1, ticker, Side::BID, 100, Money(10));
    auto increase = make_order(2, ticker, Side::BID, 100, Money(20));
    auto closing = make_order(3, ticker, Side::ASK, 100, Money(30));
    accept(*opening);
    accept(*increase);
    accept(*closing);
    fill(*opening, 100, time_from_string("2026-11-21 10:01:00"));
    fill(*increase, 100, time_from_string("2026-11-21 10:02:00"));
    fill(*closing, 100, time_from_string("2026-11-21 10:03:00"));
    auto orders = std::make_shared<ArrayListModel<std::shared_ptr<Order>>>();
    orders->push(closing);
    orders->push(opening);
    auto model = ControllerPortfolioModel(
      orders, fixture.m_market_data.get_registry_client());
    REQUIRE(model.get(0).m_inventory.m_position.m_quantity == 0);
    REQUIRE(model.get(0).m_inventory.m_gross_profit_and_loss == Money(2000));
    orders->push(increase);
    REQUIRE(model.get(0).m_inventory.m_position.m_quantity == 100);
    REQUIRE(model.get(0).m_inventory.m_position.m_cost_basis == Money(1500));
    REQUIRE(model.get(0).m_inventory.m_gross_profit_and_loss == Money(1500));
    orders->remove(2);
    REQUIRE(model.get(0).m_inventory.m_position.m_quantity == 0);
    REQUIRE(model.get(0).m_inventory.m_gross_profit_and_loss == Money(2000));
    auto replacement = make_order(4, ticker, Side::BID, 100, Money(12));
    accept(*replacement);
    fill(*replacement, 100, time_from_string("2026-11-21 10:01:00"));
    orders->set(1, replacement);
    REQUIRE(model.get(0).m_inventory.m_position.m_quantity == 0);
    REQUIRE(model.get(0).m_inventory.m_gross_profit_and_loss == Money(1800));
  }

  UI_TEST_CASE("order_list_snapshot") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("RY.TSX");
    fixture.publish(ticker, Money(12), Money(13),
      time_from_string("2026-11-21 10:00:00"));
    auto inventory = Inventory(Position(ticker, CAD, 100, Money(1000)),
      Money::ZERO, Money::ZERO, 100, 1);
    auto initial = Portfolio(TrueAverageBookkeeper(std::vector{inventory}));
    auto orders = std::make_shared<ArrayListModel<std::shared_ptr<Order>>>();
    auto model = ControllerPortfolioModel(
      orders, initial, fixture.m_market_data.get_registry_client());
    REQUIRE(model.get(0).m_inventory == inventory);
    auto closing = make_order(1, ticker, Side::ASK, 20, Money(11));
    auto opening = make_order(2, ticker, Side::BID, 10, Money(12));
    accept(*closing);
    accept(*opening);
    fill(*closing, 20, time_from_string("2026-11-21 10:01:00"));
    fill(*opening, 10, time_from_string("2026-11-21 10:02:00"));
    auto operations = std::deque<PortfolioModel::Operation>();
    model.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    orders->transact([&] {
      orders->push(opening);
      orders->transact([&] {
        orders->push(closing);
      });
      REQUIRE(model.get(0).m_inventory == inventory);
    });
    REQUIRE(model.get(0).m_inventory.m_position.m_quantity == 90);
    REQUIRE(model.get(0).m_inventory.m_position.m_cost_basis == Money(920));
    REQUIRE(model.get(0).m_inventory.m_gross_profit_and_loss == Money(20));
    require_list_transaction<PortfolioUpdateEntry>(operations,
      {PortfolioModel::PreRemoveOperation(0),
       PortfolioModel::RemoveOperation(0), PortfolioModel::AddOperation(0)});
    operations.clear();
    orders->transact([&] {
      orders->remove(1);
      orders->remove(0);
    });
    REQUIRE(model.get(0).m_inventory == inventory);
    require_list_transaction<PortfolioUpdateEntry>(operations,
      {PortfolioModel::PreRemoveOperation(0),
       PortfolioModel::RemoveOperation(0), PortfolioModel::AddOperation(0)});
    QApplication::processEvents();
    REQUIRE(model.get(0).m_inventory == inventory);
    REQUIRE(initial.get_bookkeeper().get_inventory(ticker) == inventory);
  }
}
