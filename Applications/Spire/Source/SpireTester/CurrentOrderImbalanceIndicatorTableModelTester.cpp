#define DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS

#include <doctest/doctest.h>
#include <Beam/Threading/TimerBox.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp>
#include <Beam/TimeServiceTests/TestTimeClient.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include "Spire/OrderImbalanceIndicator/CurrentOrderImbalanceIndicatorTableModel.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::TimeService::Tests;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

#define M(x, y) if(x) printf("Line: %d\t%s\n", __LINE__, y)

namespace {
  auto make_imbalance(const auto& symbol, auto timestamp) {
    return OrderImbalance(Security(symbol, DefaultCountries::US()), Side::ASK,
      Quantity(timestamp), Money(timestamp), from_time_t(timestamp));
  }

  auto row_imbalance(const auto& model, auto row) {
    auto timestamp =
      ptime(model.get<date>(row, 5), model.get<time_duration>(row, 6));
    return OrderImbalance(model.get<Security>(row, 0), model.get<Side>(row, 1),
      model.get<Quantity>(row, 2), model.get<Money>(row, 3), timestamp);
  }

  auto rows_equal(
      const auto& model, const std::vector<OrderImbalance>& imbalances) {
    if(model.get_row_size() != imbalances.size()) {
      return false;
    }
    auto model_imbalances = [&] {
      auto imbalances = std::vector<OrderImbalance>();
      for(auto i = 0; i < model.get_row_size(); ++i) {
        imbalances.push_back(row_imbalance(model, i));
      }
      return imbalances;
    }();
    return std::is_permutation(model_imbalances.begin(), model_imbalances.end(),
      imbalances.begin());
  }

  template<typename... F>
  decltype(auto) test_operation(
      const TableModel::Operation& operation, F&&... f) {
    return visit(
      operation, std::forward<F>(f)..., [] (const auto&) { REQUIRE(false); });
  }

  void wait_until(const std::function<bool ()>& predicate) {
    while(!predicate()) {
      QApplication::processEvents(QEventLoop::WaitForMoreEvents);
      QCoreApplication::sendPostedEvents();
    }
  }

  auto A100 = make_imbalance("A", 100);
  auto A300 = make_imbalance("A", 300);
  auto B100 = make_imbalance("B", 100);
  auto B150 = make_imbalance("B", 150);
  auto B350 = make_imbalance("B", 350);
  auto B400 = make_imbalance("B", 400);
  auto C100 = make_imbalance("C", 100);
  auto C110 = make_imbalance("C", 110);
  auto D100 = make_imbalance("D", 100);
}

TEST_SUITE("CurrentOrderImbalanceIndicatorTableModel") {
  TEST_CASE("expiring") {
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
    REQUIRE(row_imbalance(model, 0) == A100);
    REQUIRE(operations.size() == 1);
    test_operation(operations.front(),
      [&] (const TableModel::AddOperation& operation) {
        REQUIRE(operation.m_index == 0);
      });
    operations.pop_front();
    source->publish(B150);
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(rows_equal(model, {A100, B150}));
    REQUIRE(operations.size() == 1);
    auto row = 0;
    test_operation(operations.front(),
      [&] (const TableModel::AddOperation& operation) {
        if(row_imbalance(model, 0) == B150) {
          REQUIRE(operation.m_index == 0);
          row = 1;
        } else {
          REQUIRE(operation.m_index == 1);
          row = 0;
        }
      });
    operations.pop_front();
    environment.AdvanceTime(seconds(300));
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(row_imbalance(model, 0) == B150);
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
    REQUIRE(row_imbalance(model, 0) == A300);
    REQUIRE(operations.size() == 1);
    test_operation(operations.front(),
      [&] (const TableModel::AddOperation& operation) {
        REQUIRE(operation.m_index == 0);
      });
    operations.pop_front();
    source->publish(B350);
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(rows_equal(model, {A300, B350}));
    REQUIRE(operations.size() == 1);
    test_operation(operations.front(),
      [&] (const TableModel::AddOperation& operation) {
        if(row_imbalance(model, 0) == B350) {
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
    REQUIRE(row_imbalance(model, 0) == B350);
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
    REQUIRE(row_imbalance(model, 0) == B400);
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

  TEST_CASE("load_operations") {
    run_test([] {
      auto environment = TimeServiceTestEnvironment(from_time_t(0));
      auto timer_factory = CurrentOrderImbalanceIndicatorTableModel::TimerFactory(
        [&] (auto duration) {
          return
            TimerBox(std::make_unique<TestTimer>(duration, Ref(environment)));
        });
      auto source = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      source->publish(A100);
      source->publish(B150);
      source->publish(C100);
      auto model = CurrentOrderImbalanceIndicatorTableModel(seconds(200),
        TimeClientBox(std::make_unique<TestTimeClient>(Ref(environment))),
        timer_factory, source);
      auto operations_count = 0;
      auto connection = scoped_connection(model.connect_operation_signal(
        [&] (const auto& operation) {
          ++operations_count;
          auto transaction = get<TableModel::Transaction>(&operation);
          REQUIRE(transaction != nullptr);
          REQUIRE(transaction->m_operations.size() == 3);
          REQUIRE(rows_equal(model, {A100, B150, C100}));
          auto& operations = transaction->m_operations;
          for(auto& add : operations) {
            test_operation(add, [&] (const TableModel::AddOperation&) {});
          }
        }));
      wait_until([&] { return operations_count == 1; });
      REQUIRE(operations_count == 1);
    });
  }

  TEST_CASE("older_published_imbalances") {
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
    source->publish(B350);
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(row_imbalance(model, 0) == B350);
    environment.AdvanceTime(seconds(250));
    source->publish(C110);
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(rows_equal(model, {B350, C110}));
    environment.AdvanceTime(seconds(25));
    source->publish(A100);
    REQUIRE(model.get_row_size() == 3);
    REQUIRE(rows_equal(model, {B350, C110, A100}));
    environment.AdvanceTime(seconds(25));
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(rows_equal(model, {B350, C110}));
    environment.AdvanceTime(seconds(25));
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(row_imbalance(model, 0) == B350);
    source->publish(B150);
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(row_imbalance(model, 0) == B350);
    environment.AdvanceTime(seconds(225));
    REQUIRE(model.get_row_size() == 0);
  }

  TEST_CASE("coincident_timestamps") {
    run_test([] {
      auto environment = TimeServiceTestEnvironment(from_time_t(0));
      auto timer_factory = CurrentOrderImbalanceIndicatorTableModel::TimerFactory(
        [&] (auto duration) {
          return
            TimerBox(std::make_unique<TestTimer>(duration, Ref(environment)));
        });
      auto source = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      source->publish(A100);
      source->publish(B100);
      source->publish(C100);
      source->publish(D100);
      auto model = CurrentOrderImbalanceIndicatorTableModel(seconds(200),
        TimeClientBox(std::make_unique<TestTimeClient>(Ref(environment))),
        timer_factory, source);
      auto operations = std::deque<TableModel::Operation>();
      auto connection = scoped_connection(model.connect_operation_signal(
        [&] (const auto& operation) { operations.push_back(operation);
          environment.AdvanceTime(seconds(10)); }));
      wait_until([&] { return model.get_row_size() == 4; });
      REQUIRE(model.get_row_size() == 4);
      environment.AdvanceTime(seconds(300));
      REQUIRE(model.get_row_size() == 0);
    });
  }
}
