#include <doctest/doctest.h>
#include <Beam/Threading/TimerBox.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp>
#include <Beam/TimeServiceTests/TestTimeClient.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include "Spire/OrderImbalanceIndicator/CurrentOrderImbalanceIndicatorTableModel.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::TimeService::Tests;
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
  auto B400 = make_imbalance("B", 400);
}

TEST_SUITE("CurrentOrderImbalanceIndicatorTableModel") {
  TEST_CASE("offset") {
    auto environment = TimeServiceTestEnvironment(from_time_t(0));
    auto timer_factory = CurrentOrderImbalanceIndicatorTableModel::TimerFactory(
      [&] (auto duration) {
        return
          TimerBox(std::make_unique<TestTimer>(duration, Ref(environment)));
      });
    auto source = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    auto model = CurrentOrderImbalanceIndicatorTableModel(seconds(200),
      TimeClientBox(std::make_unique<TestTimeClient>(Ref(environment))),
      timer_factory, source);
    auto operations = std::deque<TableModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); }));
    source->publish(A100);
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(row_equals(model, 0, A100));
    REQUIRE(operations.size() == 1);
    test_operation(operations.front(),
      [&] (const TableModel::AddOperation& operation) {
        REQUIRE(operation.m_index == 0);
      });
    operations.pop_front();
    source->publish(B150);
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(rows_equal(model, A100, B150));
    REQUIRE(operations.size() == 1);
    auto row = 0;
    test_operation(operations.front(),
      [&] (const TableModel::AddOperation& operation) {
        if(row_equals(model, 0, B150)) {
          REQUIRE(operation.m_index == 0);
          row = 1;
        } else {
          REQUIRE(operation.m_index == 1);
          row = 0;
        }
      });
    operations.pop_front();
    environment.AdvanceTime(seconds(100));
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(row_equals(model, 0, B150));
    REQUIRE(operations.size() == 1);
    test_operation(operations.front(),
      [&] (const TableModel::RemoveOperation& operation) {
        REQUIRE(operation.m_index == row);
      });
    operations.pop_front();
    environment.AdvanceTime(seconds(50));
    REQUIRE(model.get_row_size() == 0);
    REQUIRE(operations.size() == 1);
    test_operation(operations.front(),
      [&] (const TableModel::RemoveOperation& operation) {
        REQUIRE(operation.m_index == 0);
      });
    operations.pop_front();
    source->publish(A300);
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(row_equals(model, 0, A300));
    REQUIRE(operations.size() == 1);
    test_operation(operations.front(),
      [&] (const TableModel::AddOperation& operation) {
        REQUIRE(operation.m_index == 0);
      });
    operations.pop_front();
    source->publish(B350);
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(rows_equal(model, A300, B350));
    REQUIRE(operations.size() == 1);
    test_operation(operations.front(),
      [&] (const TableModel::AddOperation& operation) {
        if(row_equals(model, 0, B350)) {
          REQUIRE(operation.m_index == 0);
          row = 1;
        } else {
          REQUIRE(operation.m_index == 1);
          row = 0;
        }
      });
    operations.pop_front();
    environment.AdvanceTime(seconds(150));
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(row_equals(model, 0, B350));
    REQUIRE(operations.size() == 1);
    test_operation(operations.front(),
      [&] (const TableModel::RemoveOperation& operation) {
        REQUIRE(operation.m_index == row);
      });
    operations.pop_front();
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        operations.push_back(operation);
        REQUIRE(operations.size() == 1);
        auto transaction = get<TableModel::Transaction>(&operation);
        REQUIRE(transaction != nullptr);
        REQUIRE(transaction->m_operations.size() == 4);
        auto& operations = transaction->m_operations;
        auto columns = std::queue<int>({2, 3, 4, 6});
        for(auto i = 0; i < 4; ++i) {
          auto operation = get<TableModel::UpdateOperation>(&operations[i]);
          REQUIRE(operation);
          REQUIRE(operation->m_column == columns.front());
          REQUIRE(operation->m_row == 0);
          columns.pop();
        }
      });
    source->publish(B400);
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(row_equals(model, 0, B400));
    operations.pop_front();
    connection = model.connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); });
    environment.AdvanceTime(seconds(100));
    REQUIRE(model.get_row_size() == 0);
    REQUIRE(operations.size() == 1);
    test_operation(operations.front(),
      [&] (const TableModel::RemoveOperation& operation) {
        REQUIRE(operation.m_index == 0);
      });
    operations.pop_front();
  }
}
