#include <deque>
#include <thread>
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Spire/Blotter/OrderExecutionReportsModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_order(OrderId id) {
    auto fields = OrderFields();
    fields.m_quantity = 100;
    return std::make_shared<PrimitiveOrder>(
      OrderInfo(fields, id, time_from_string("2026-11-20 10:00:00")));
  }
}

TEST_SUITE("OrderExecutionReportsModel") {
  UI_TEST_CASE("empty") {
    auto orders = std::make_shared<ArrayListModel<OrdersModel::Type>>();
    auto model = OrderExecutionReportsModel(orders);
    REQUIRE(model.get_size() == 0);
    REQUIRE_THROWS_AS(model.get(0), std::out_of_range);
  }

  UI_TEST_CASE("existing_reports") {
    auto first = make_order(1);
    auto second = make_order(2);
    Nexus::Tests::accept(*first);
    Nexus::Tests::fill(*first, Money(12), 100);
    Nexus::Tests::reject(*second);
    auto orders = std::make_shared<ArrayListModel<OrdersModel::Type>>(
      std::vector<OrdersModel::Type>{first, second});
    auto model = OrderExecutionReportsModel(orders);
    REQUIRE(model.get_size() == 5);
    auto index = 0;
    for(auto& order : std::as_const(*orders)) {
      auto reports = order->get_publisher().get_snapshot();
      for(auto& report : *reports) {
        REQUIRE(model.get(index).m_order == order);
        REQUIRE(model.get(index).m_report == report);
        ++index;
      }
    }
    QApplication::processEvents();
    REQUIRE(model.get_size() == 5);
  }

  UI_TEST_CASE("published_reports") {
    auto order = make_order(1);
    auto orders = std::make_shared<ArrayListModel<OrdersModel::Type>>(
      std::vector<OrdersModel::Type>{order});
    auto model = OrderExecutionReportsModel(orders);
    auto operations = std::deque<ExecutionReportsModel::Operation>();
    model.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    auto report = make_update(order->get_publisher().get_snapshot()->back(),
      OrderStatus::PARTIALLY_FILLED, time_from_string("2026-11-20 10:01:00"));
    report.m_last_quantity = 20;
    report.m_last_price = Money(12);
    report.m_execution_fee = Money::CENT;
    report.m_text = "Partial fill";
    auto thread = std::thread([&] {
      order->update(report);
      order->update(report);
    });
    thread.join();
    REQUIRE(model.get_size() == 1);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(1).m_order == order);
    REQUIRE(model.get(1).m_report == report);
    require_list_transaction<ExecutionReportEntry>(
      operations, {ExecutionReportsModel::AddOperation(1)});
    operations.clear();
    ++report.m_sequence;
    order->update(report);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get(2).m_report == report);
    require_list_transaction<ExecutionReportEntry>(
      operations, {ExecutionReportsModel::AddOperation(2)});
  }

  UI_TEST_CASE("order_changes") {
    auto first = make_order(1);
    auto second = make_order(2);
    auto third = make_order(3);
    auto orders = std::make_shared<ArrayListModel<OrdersModel::Type>>();
    auto model = OrderExecutionReportsModel(orders);
    orders->push(first);
    orders->push(second);
    orders->insert(OrdersModel::Type(third), 0);
    orders->move(0, 2);
    orders->move(1, 0);
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get(0).m_order == first);
    REQUIRE(model.get(1).m_order == second);
    REQUIRE(model.get(2).m_order == third);
    Nexus::Tests::accept(*second);
    orders->remove(0);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(0).m_order == first);
    REQUIRE(model.get(1).m_order == third);
    Nexus::Tests::accept(*first);
    orders->set(0, second);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get(0).m_order == third);
    REQUIRE(model.get(1).m_order == second);
    REQUIRE(model.get(2).m_report.m_status == OrderStatus::NEW);
    Nexus::Tests::reject(*first);
    Nexus::Tests::accept(*third);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 4);
    REQUIRE(model.get(3).m_order == third);
  }

  UI_TEST_CASE("duplicate_orders") {
    auto first = make_order(1);
    auto second = make_order(1);
    Nexus::Tests::accept(*second);
    auto orders = std::make_shared<ArrayListModel<OrdersModel::Type>>(
      std::vector<OrdersModel::Type>{first, first, second});
    auto model = OrderExecutionReportsModel(orders);
    REQUIRE(model.get_size() == 2);
    orders->remove(2);
    REQUIRE(model.get_size() == 1);
    orders->remove(0);
    REQUIRE(model.get_size() == 1);
    Nexus::Tests::accept(*first);
    orders->remove(0);
    REQUIRE(model.get_size() == 0);
    orders->push(first);
    REQUIRE(model.get_size() == 2);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 2);
    orders->set(0, second);
    REQUIRE(model.get_size() == 2);
    Nexus::Tests::fill(*second, Money(12), 100);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get(2).m_order == second);
    REQUIRE(model.get(2).m_report.m_status == OrderStatus::FILLED);
    orders->remove(0);
    REQUIRE(model.get_size() == 0);
  }

  UI_TEST_CASE("order_transactions") {
    auto first = make_order(1);
    auto second = make_order(2);
    auto orders = std::make_shared<ArrayListModel<OrdersModel::Type>>();
    auto model = OrderExecutionReportsModel(orders);
    auto operations = std::deque<ExecutionReportsModel::Operation>();
    model.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
      if(auto removal =
          std::get_if<ExecutionReportsModel::PreRemoveOperation>(&operation)) {
        REQUIRE(model.get(removal->m_index).m_order);
      }
    });
    orders->transact([&] {
      orders->push(first);
      orders->push(second);
    });
    require_list_transaction<ExecutionReportEntry>(operations, {
      ExecutionReportsModel::AddOperation(0),
      ExecutionReportsModel::AddOperation(1)});
    operations.clear();
    orders->set(0, first);
    REQUIRE(operations.empty());
    orders->transact([&] {
      orders->remove(1);
      orders->remove(0);
    });
    require_list_transaction<ExecutionReportEntry>(operations, {
      ExecutionReportsModel::PreRemoveOperation(1),
      ExecutionReportsModel::RemoveOperation(1),
      ExecutionReportsModel::PreRemoveOperation(0),
      ExecutionReportsModel::RemoveOperation(0)});
    REQUIRE(model.get_size() == 0);
  }

  UI_TEST_CASE("destruction_with_queued_reports") {
    auto order = make_order(1);
    auto orders = std::make_shared<ArrayListModel<OrdersModel::Type>>(
      std::vector<OrdersModel::Type>{order});
    auto operations = std::deque<ExecutionReportsModel::Operation>();
    {
      auto model = OrderExecutionReportsModel(orders);
      model.connect_operation_signal([&] (const auto& operation) {
        operations.push_back(operation);
      });
      Nexus::Tests::accept(*order);
    }
    Nexus::Tests::fill(*order, Money(12), 100);
    QApplication::processEvents();
    REQUIRE(operations.empty());
  }
}
