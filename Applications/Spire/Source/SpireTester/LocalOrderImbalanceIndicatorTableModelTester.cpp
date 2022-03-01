#include <doctest/doctest.h>
#include <queue>
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorTableModel.hpp"

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

  template<typename... F>
  decltype(auto) test_operation(
      const TableModel::Operation& operation, F&&... f) {
    return visit(
      operation, std::forward<F>(f)..., [] (const auto&) { REQUIRE(false); });
  }

  const auto A100 = make_imbalance("A", 100);
  const auto A300 = make_imbalance("A", 300);
  const auto B100 = make_imbalance("B", 100);
  const auto B350 = make_imbalance("B", 350);
}

TEST_SUITE("LocalOrderImbalanceIndicatorTableModel") {
  TEST_CASE("add") {
    auto model = LocalOrderImbalanceIndicatorTableModel();
    auto operations = std::deque<TableModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); }));
    model.add(A100);
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(row_equals(model, 0, A100));
    REQUIRE(operations.size() == 1);
    test_operation(operations.front(),
      [&] (const TableModel::AddOperation& operation) {
        REQUIRE(operation.m_index == 0);
      });
    operations.pop_front();
    model.add(B100);
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(row_equals(model, 0, A100));
    REQUIRE(row_equals(model, 1, B100));
    REQUIRE(operations.size() == 1);
    test_operation(operations.front(),
      [&] (const TableModel::AddOperation& operation) {
        REQUIRE(operation.m_index == 1);
      });
    operations.pop_front();
    model.add(B100);
    REQUIRE(operations.empty());
    auto row = 1;
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        operations.push_back(operation);
        auto transaction = get<TableModel::Transaction>(&operation);
        REQUIRE(transaction != nullptr);
        REQUIRE(transaction->m_operations.size() == 4);
        auto& operations = transaction->m_operations;
        auto columns = std::queue<int>({2, 3, 4, 6});
        for(auto i = 0; i < 4; ++i) {
          auto operation = get<TableModel::UpdateOperation>(&operations[i]);
          REQUIRE(operation);
          REQUIRE(operation->m_column == columns.front());
          REQUIRE(operation->m_row == row);
          columns.pop();
        }
      });
    model.add(B350);
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(row_equals(model, 0, A100));
    REQUIRE(row_equals(model, 1, B350));
    operations.pop_front();
    row = 0;
    model.add(A300);
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(row_equals(model, 0, A300));
    REQUIRE(row_equals(model, 1, B350));
  }

  TEST_CASE("remove") {
    auto model = LocalOrderImbalanceIndicatorTableModel();
    model.add(A100);
    model.add(B350);
    auto operations = std::deque<TableModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); }));
    model.remove(A100.m_security);
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(row_equals(model, 0, B350));
    test_operation(operations.front(),
      [&] (const TableModel::RemoveOperation& operation) {
        REQUIRE(operation.m_index == 0);
      });
    model.remove(B350.m_security);
    REQUIRE(model.get_row_size() == 0);
    test_operation(operations.front(),
      [&] (const TableModel::RemoveOperation& operation) {
        REQUIRE(operation.m_index == 0);
      });
  }
}
