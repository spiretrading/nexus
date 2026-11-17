#include <thread>
#include <QLayout>
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Spire/Blotter/OrderLogView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto TIME_COLUMN = 0;
  const auto STATUS_COLUMN = 2;

  struct Fixture {
    std::shared_ptr<ArrayListModel<OrdersModel::Type>> m_orders;
    std::unique_ptr<OrderLogView> m_view;
    std::shared_ptr<TableModel> m_table;

    Fixture()
        : m_orders(std::make_shared<ArrayListModel<OrdersModel::Type>>()) {
      initialize_resources();
    }

    void open() {
      m_view = std::make_unique<OrderLogView>(m_orders);
      auto table_view =
        dynamic_cast<TableView*>(m_view->layout()->itemAt(0)->widget());
      REQUIRE(table_view);
      m_table = table_view->get_table();
    }

    auto status(int row) const {
      return m_table->get<OrderStatus>(row, STATUS_COLUMN);
    }
  };

  auto make_order(OrderId id) {
    auto fields = OrderFields();
    fields.m_quantity = 100;
    return std::make_shared<PrimitiveOrder>(OrderInfo(fields, id,
      time_from_string("2026-11-17 10:00:00")));
  }
}

TEST_SUITE("OrderLogView") {
  UI_TEST_CASE("time_and_status") {
    auto fixture = Fixture();
    auto order = make_order(1);
    Nexus::Tests::accept(*order);
    fixture.m_orders->push(order);
    fixture.open();
    REQUIRE(fixture.m_table->get<ptime>(0, TIME_COLUMN) ==
      order->get_info().m_timestamp);
    REQUIRE(fixture.status(0) == OrderStatus::NEW);
    auto updates = std::vector<TableModel::UpdateOperation>();
    auto connection = fixture.m_table->connect_operation_signal(
      [&] (const auto& operation) {
        if(auto update = std::get_if<TableModel::UpdateOperation>(&operation)) {
          updates.push_back(*update);
        }
      });
    auto thread = std::thread([&] {
      Nexus::Tests::fill(*order, Money(10), 40);
    });
    thread.join();
    REQUIRE(fixture.status(0) == OrderStatus::NEW);
    QApplication::processEvents();
    REQUIRE(fixture.status(0) == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front().m_row == 0);
    REQUIRE(updates.front().m_column == STATUS_COLUMN);
    REQUIRE(std::any_cast<OrderStatus>(updates.front().m_previous) ==
      OrderStatus::NEW);
    REQUIRE(std::any_cast<OrderStatus>(updates.front().m_value) ==
      OrderStatus::PARTIALLY_FILLED);
    Nexus::Tests::fill(*order, Money(11), 60);
    QApplication::processEvents();
    REQUIRE(fixture.status(0) == OrderStatus::FILLED);
    REQUIRE(fixture.m_table->get<ptime>(0, TIME_COLUMN) ==
      order->get_info().m_timestamp);
  }

  UI_TEST_CASE("price_sorting") {
    const auto PRICE_COLUMN = 9;
    auto fixture = Fixture();
    auto add_order = [&] (OrderType type, Money price) {
      auto fields = OrderFields();
      fields.m_type = type;
      fields.m_price = price;
      fixture.m_orders->push(std::make_shared<PrimitiveOrder>(
        OrderInfo(fields, fixture.m_orders->get_size() + 1,
          time_from_string("2026-11-17 10:00:00"))));
    };
    add_order(OrderType::LIMIT, Money(10));
    add_order(OrderType::MARKET, Money(0));
    add_order(OrderType::LIMIT, Money(2));
    fixture.open();
    auto sorted = SortedTableModel(fixture.m_table);
    auto price = [&] (int row) {
      return to_text(sorted.at(row, PRICE_COLUMN));
    };
    sorted.set_column_order(
      {{PRICE_COLUMN, SortedTableModel::Ordering::ASCENDING}});
    REQUIRE(price(0).isEmpty());
    REQUIRE(price(1) == to_text(Money(2)));
    REQUIRE(price(2) == to_text(Money(10)));
    sorted.set_column_order(
      {{PRICE_COLUMN, SortedTableModel::Ordering::DESCENDING}});
    REQUIRE(price(0) == to_text(Money(10)));
    REQUIRE(price(1) == to_text(Money(2)));
    REQUIRE(price(2).isEmpty());
  }

  UI_TEST_CASE("order_list_changes") {
    auto fixture = Fixture();
    fixture.open();
    auto first = make_order(1);
    auto second = make_order(2);
    fixture.m_orders->push(first);
    fixture.m_orders->insert(OrdersModel::Type(second), 0);
    REQUIRE(fixture.status(0) == OrderStatus::PENDING_NEW);
    Nexus::Tests::reject(*first);
    fixture.m_orders->move(1, 0);
    QApplication::processEvents();
    REQUIRE(fixture.status(0) == OrderStatus::REJECTED);
    REQUIRE(fixture.status(1) == OrderStatus::PENDING_NEW);
    Nexus::Tests::accept(*second);
    fixture.m_table->remove(0);
    REQUIRE(fixture.m_orders->get_size() == 1);
    QApplication::processEvents();
    REQUIRE(fixture.status(0) == OrderStatus::NEW);
    Nexus::Tests::set_order_status(*second, OrderStatus::CANCELED);
    auto replacement = make_order(3);
    Nexus::Tests::reject(*replacement);
    fixture.m_orders->set(0, replacement);
    QApplication::processEvents();
    REQUIRE(fixture.status(0) == OrderStatus::REJECTED);
    fixture.m_orders->push(second);
    REQUIRE(fixture.status(1) == OrderStatus::CANCELED);
    auto removed = make_order(4);
    fixture.m_orders->insert(OrdersModel::Type(removed), 0);
    Nexus::Tests::accept(*removed);
    fixture.m_orders->remove(0);
    QApplication::processEvents();
    REQUIRE(fixture.status(0) == OrderStatus::REJECTED);
    REQUIRE(fixture.status(1) == OrderStatus::CANCELED);
  }

  UI_TEST_CASE("order_transactions") {
    auto fixture = Fixture();
    fixture.open();
    auto operations = std::vector<TableModel::Operation>();
    auto connection = fixture.m_table->connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); });
    fixture.m_orders->transact([&] {
      fixture.m_orders->push(make_order(1));
      fixture.m_orders->push(make_order(2));
    });
    REQUIRE(operations.size() == 4);
    REQUIRE(
      std::holds_alternative<TableModel::StartTransaction>(operations[0]));
    REQUIRE(std::get<TableModel::AddOperation>(operations[1]).m_index == 0);
    REQUIRE(std::get<TableModel::AddOperation>(operations[2]).m_index == 1);
    REQUIRE(std::holds_alternative<TableModel::EndTransaction>(operations[3]));
    REQUIRE(fixture.m_table->get_row_size() == 2);
    REQUIRE(fixture.m_table->set(0, STATUS_COLUMN, OrderStatus::FILLED) ==
      QValidator::Invalid);
    REQUIRE(fixture.status(0) == OrderStatus::PENDING_NEW);
  }

  UI_TEST_CASE("destruction_with_queued_report") {
    auto order = make_order(1);
    {
      auto fixture = Fixture();
      fixture.m_orders->push(order);
      fixture.open();
      Nexus::Tests::accept(*order);
    }
    Nexus::Tests::fill(*order, Money(10), 100);
    QApplication::processEvents();
    REQUIRE(order->get_publisher().get_snapshot()->back().m_status ==
      OrderStatus::FILLED);
  }
}
