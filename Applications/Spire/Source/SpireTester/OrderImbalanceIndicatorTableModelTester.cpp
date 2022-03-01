#include <doctest/doctest.h>
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorTableModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_imbalance(const auto& symbol, auto timestamp) {
    return OrderImbalance(Security(symbol, DefaultCountries::US()), Side::ASK,
      Quantity(timestamp), Money(timestamp), from_time_t(timestamp));
  }

  auto row_equals(const auto& model, auto row, auto imbalance) {
    return model.get<Security>(row, 0) == imbalance.m_security &&
      model.get<Side>(row, 1) == imbalance.m_side &&
      model.get<Quantity>(row, 2) == imbalance.m_size &&
      model.get<Money>(row, 3) == imbalance.m_referencePrice &&
      model.get<Money>(row, 4) ==
        imbalance.m_size * imbalance.m_referencePrice &&
      model.get<date>(row, 5) == imbalance.m_timestamp.date() &&
      model.get<time_duration>(row, 6) == imbalance.m_timestamp.time_of_day();
  }

  auto rows_equal(
      const auto& model, const auto& imbalance1, const auto& imbalance2) {
    if(model.get_row_size() != 2) {
      return false;
    }
    return
      row_equals(model, 0, imbalance1) && row_equals(model, 1, imbalance2) ||
      row_equals(model, 0, imbalance2) && row_equals(model, 1, imbalance1);
  }

  template<typename... F>
  decltype(auto) test_operation(
      const TableModel::Operation& operation, F&&... f) {
    return visit(
      operation, std::forward<F>(f)..., [] (const auto&) { REQUIRE(false); });
  }

  auto A100 = make_imbalance("A", 100);
  auto A300 = make_imbalance("A", 300);
  auto B150 = make_imbalance("B", 150);
  auto B350 = make_imbalance("B", 350);
}

TEST_SUITE("OrderImbalanceIndicatorTableModel") {
  TEST_CASE("closed_interval") {
    run_test([] {
      auto source = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      source->publish(A100);
      auto model = OrderImbalanceIndicatorTableModel(
        TimeInterval::closed(from_time_t(100), from_time_t(300)), source);
      auto operations = std::deque<TableModel::Operation>();
      auto connection = scoped_connection(model.connect_operation_signal(
        [&] (const auto& operation) { operations.push_back(operation); }));
      wait_until([&] { return model.get_row_size() == 1; });
      REQUIRE(model.get_row_size() == 1);
      REQUIRE(row_equals(model, 0, A100));
      REQUIRE(operations.size() == 1);
      test_operation(operations.front(),
        [] (const TableModel::AddOperation& operation) {
          REQUIRE(operation.m_index == 0);
        });
      operations.pop_front();
      source->publish(B150);
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(rows_equal(model, A100, B150));
      REQUIRE(operations.size() == 1);
      test_operation(operations.front(),
        [&] (const TableModel::AddOperation& operation) {
          if(row_equals(model, 0, B150)) {
            REQUIRE(operation.m_index == 0);
          } else {
            REQUIRE(operation.m_index == 1);
          }
        });
      operations.pop_front();
      source->publish(B350);
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(rows_equal(model, A100, B150));
      REQUIRE(operations.empty());
      connection = model.connect_operation_signal(
        [&] (const TableModel::Operation& operation) {
          operations.push_back(operation);
          auto transaction = get<TableModel::Transaction>(&operation);
          REQUIRE(transaction != nullptr);
          REQUIRE(transaction->m_operations.size() == 4);
          auto& operations = transaction->m_operations;
          auto row = get<TableModel::UpdateOperation>(operations.front()).m_row;
          auto columns = std::queue<int>({2, 3, 4, 6});
          for(auto i = 0; i < 4; ++i) {
            auto operation = get<TableModel::UpdateOperation>(&operations[i]);
            REQUIRE(operation);
            REQUIRE(operation->m_column == columns.front());
            REQUIRE(operation->m_row == row);
            columns.pop();
          }
        });
      source->publish(A300);
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(rows_equal(model, A300, B150));
      REQUIRE(operations.size() == 1);
      operations.pop_front();
    });
  }

  TEST_CASE("open_interval") {
    run_test([] {
      auto source = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      source->publish(B150);
      auto model = OrderImbalanceIndicatorTableModel(
        TimeInterval::open(from_time_t(100), from_time_t(350)), source);
      auto operations = std::deque<TableModel::Operation>();
      auto connection = scoped_connection(model.connect_operation_signal(
        [&] (const auto& operation) { operations.push_back(operation); }));
      wait_until([&] { return model.get_row_size() == 1; });
      REQUIRE(model.get_row_size() == 1);
      REQUIRE(row_equals(model, 0, B150));
      REQUIRE(operations.size() == 1);
      test_operation(operations.front(),
        [] (const TableModel::AddOperation& operation) {
          REQUIRE(operation.m_index == 0);
        });
      operations.pop_front();
      source->publish(A100);
      REQUIRE(model.get_row_size() == 1);
      REQUIRE(row_equals(model, 0, B150));
      REQUIRE(operations.empty());
      source->publish(B350);
      REQUIRE(model.get_row_size() == 1);
      REQUIRE(row_equals(model, 0, B150));
      REQUIRE(operations.empty());
      source->publish(A300);
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(rows_equal(model, B150, A300));
      REQUIRE(operations.size() == 1);
      test_operation(operations.front(),
        [&] (const TableModel::AddOperation& operation) {
          if(row_equals(model, 0, A300)) {
            REQUIRE(operation.m_index == 0);
          } else {
            REQUIRE(operation.m_index == 1);
          }
        });
      operations.pop_front();
    });
  }
}
