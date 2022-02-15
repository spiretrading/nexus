#include <doctest/doctest.h>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorTableModel.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam::TimeService;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  struct TableOperationLog {
    std::vector<TableModel::AddOperation> m_adds;
    std::vector<TableModel::RemoveOperation> m_removes;
    std::vector<TableModel::MoveOperation> m_moves;
    std::vector<TableModel::UpdateOperation> m_updates;

    std::size_t operation_count() const {
      return
        m_adds.size() + m_removes.size() + m_moves.size() + m_updates.size();
    }

    void on_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          m_adds.push_back(operation);
        },
        [&] (const TableModel::RemoveOperation& operation) {
          m_removes.push_back(operation);
        },
        [&] (const TableModel::MoveOperation& operation) {
          m_moves.push_back(operation);
        },
        [&] (const TableModel::UpdateOperation& operation) {
          m_updates.push_back(operation);
        });
    }
  };

  auto make_imbalance(const auto& symbol, auto timestamp) {
    return OrderImbalance(Security(symbol, DefaultCountries::US()), Side::ASK,
      Quantity(timestamp), Money(12.34), from_time_t(timestamp));
  }

  auto closed(auto lower, auto upper) {
    return TimeInterval::closed(from_time_t(lower), from_time_t(upper));
  }

  auto open(auto lower, auto upper) {
    return TimeInterval::open(from_time_t(lower), from_time_t(upper));
  }

  bool contains(const auto& container, const auto& value) {
    return
      std::find(container.begin(), container.end(), value) != container.end();
  }

  auto wait_until(const std::function<bool ()> expression) {
    while(!expression()) {
      QApplication::processEvents(QEventLoop::WaitForMoreEvents);
      QCoreApplication::sendPostedEvents();
    }
  }

  auto rows_equal(
      const auto& model, const auto& imbalance1, const auto& imbalance2) {
    if(model.get_row_size() != 2) {
      return false;
    }
    auto row1 = std::tie(model.get<Security>(0, 0), model.get<Quantity>(0, 2));
    auto row2 = std::tie(model.get<Security>(1, 0), model.get<Quantity>(1, 2));
    auto imb1 = std::tie(imbalance1.m_security, imbalance1.m_size);
    auto imb2 = std::tie(imbalance2.m_security, imbalance2.m_size);
    return row1 != row2 && (row1 == imb1 && row2 == imb2 ||
      row1 == imb2 && row2 == imb1);
  }

  const auto A100 = make_imbalance("A", 100);
  const auto A300 = make_imbalance("A", 300);
  const auto A500 = make_imbalance("A", 500);
  const auto B100 = make_imbalance("B", 100);
  const auto B300 = make_imbalance("B", 300);
  const auto B550 = make_imbalance("B", 550);
}

TEST_SUITE("OrderImbalanceIndicatorTableModel") {
  TEST_CASE("set_interval") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      auto model = OrderImbalanceIndicatorTableModel(
        local_model, TimeClientBox(std::make_shared<FixedTimeClient>()));
      local_model->publish(A100);
      local_model->publish(B100);
      auto operation_log = TableOperationLog{};
      model.connect_operation_signal(
        std::bind_front(&TableOperationLog::on_operation, &operation_log));
      model.set_interval(closed(0, 200));
      wait_until([&] { return operation_log.operation_count() == 2; });
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(rows_equal(model, A100, B100));
      REQUIRE(operation_log.operation_count() == 2);
      REQUIRE(operation_log.m_adds.size() == 2);
      model.set_interval(open(0, 100));
      wait_until([&] { return operation_log.operation_count() == 4; });
      REQUIRE(model.get_row_size() == 0);
      REQUIRE(operation_log.operation_count() == 4);
      REQUIRE(operation_log.m_adds.size() == 2);
      REQUIRE(operation_log.m_removes.size() == 2);
      local_model->publish(A300);
      local_model->publish(B300);
      REQUIRE(operation_log.operation_count() == 4);
      REQUIRE(model.get_row_size() == 0);
      model.set_interval(closed(200, 500));
      wait_until([&] { return operation_log.operation_count() == 6; });
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(rows_equal(model, A300, B300));
      REQUIRE(operation_log.operation_count() == 6);
      REQUIRE(operation_log.m_adds.size() == 4);
      REQUIRE(operation_log.m_removes.size() == 2);
      local_model->publish(A500);
      local_model->publish(B550);
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(rows_equal(model, A500, B300));
      REQUIRE(operation_log.operation_count() == 9);
      REQUIRE(operation_log.m_adds.size() == 4);
      REQUIRE(operation_log.m_removes.size() == 2);
      REQUIRE(operation_log.m_updates.size() == 3);
    });
  }

  TEST_CASE("set_offset") {
    run_test([] {
      auto clock = std::make_shared<FixedTimeClient>(from_time_t(200));
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      local_model->publish(A100);
      local_model->publish(B100);
      auto model =
        OrderImbalanceIndicatorTableModel(local_model, TimeClientBox(clock));
      auto operation_log = TableOperationLog{};
      model.connect_operation_signal(
        std::bind_front(&TableOperationLog::on_operation, &operation_log));
      REQUIRE(model.get_row_size() == 0);
      model.set_offset(seconds(600));
      wait_until([&] { return operation_log.operation_count() == 2; });
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(rows_equal(model, A100, B100));
      REQUIRE(operation_log.operation_count() == 2);
      REQUIRE(operation_log.m_adds.size() == 2);
      clock->SetTime(from_time_t(300));
      local_model->publish(A300);
      local_model->publish(B300);
      wait_until([&] { return operation_log.operation_count() == 8; });
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(rows_equal(model, A300, B300));
      REQUIRE(operation_log.operation_count() == 8);
      REQUIRE(operation_log.m_adds.size() == 2);
      REQUIRE(operation_log.m_updates.size() == 6);
    });
  }

  TEST_CASE("published_imbalances") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      auto model = OrderImbalanceIndicatorTableModel(
        local_model, TimeClientBox(std::make_shared<FixedTimeClient>()));
      auto operation_log = TableOperationLog{};
      model.connect_operation_signal(
        std::bind_front(&TableOperationLog::on_operation, &operation_log));
      model.set_interval(open(100, 550));
      local_model->publish(A100);
      REQUIRE(model.get_row_size() == 0);
      REQUIRE(operation_log.operation_count() == 0);
      local_model->publish(B100);
      REQUIRE(model.get_row_size() == 0);
      REQUIRE(operation_log.operation_count() == 0);
      local_model->publish(A300);
      REQUIRE(model.get_row_size() == 1);
      REQUIRE(operation_log.operation_count() == 1);
      REQUIRE(operation_log.m_adds.size() == 1);
      local_model->publish(B300);
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(operation_log.operation_count() == 2);
      REQUIRE(operation_log.m_adds.size() == 2);
      local_model->publish(A500);
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(operation_log.operation_count() == 5);
      REQUIRE(operation_log.m_adds.size() == 2);
      REQUIRE(operation_log.m_updates.size() == 3);
      local_model->publish(B550);
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(operation_log.operation_count() == 5);
    });
  }

  TEST_CASE("expired_imbalances") {
    run_test([] {
      auto clock = std::make_shared<FixedTimeClient>(from_time_t(0));
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      auto model =
        OrderImbalanceIndicatorTableModel(local_model, TimeClientBox(clock));
      auto operation_log = TableOperationLog{};
      model.connect_operation_signal(
        std::bind_front(&TableOperationLog::on_operation, &operation_log));
      local_model->publish(A100);
      local_model->publish(B100);
      model.set_offset(seconds(200));
      wait_until([&] { return operation_log.operation_count() == 2; });
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(operation_log.operation_count() == 2);
      REQUIRE(operation_log.m_adds.size() == 2);
      clock->SetTime(from_time_t(350));
      wait_until([&] { return operation_log.operation_count() == 4; });
      REQUIRE(model.get_row_size() == 0);
      REQUIRE(operation_log.operation_count() == 4);
      REQUIRE(operation_log.m_adds.size() == 2);
      REQUIRE(operation_log.m_removes.size() == 2);
      clock->SetTime(from_time_t(600));
      local_model->publish(A500);
      local_model->publish(B550);
      wait_until([&] { return operation_log.operation_count() == 6; });
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(operation_log.operation_count() == 6);
      REQUIRE(operation_log.m_adds.size() == 4);
      REQUIRE(operation_log.m_removes.size() == 2);
      clock->SetTime(from_time_t(725));
      wait_until([&] { return operation_log.operation_count() == 7; });
      REQUIRE(model.get_row_size() == 1);
      REQUIRE(operation_log.operation_count() == 7);
      REQUIRE(operation_log.m_adds.size() == 4);
      REQUIRE(operation_log.m_removes.size() == 3);
      clock->SetTime(from_time_t(775));
      wait_until([&] { return operation_log.operation_count() == 8; });
      REQUIRE(model.get_row_size() == 0);
      REQUIRE(operation_log.operation_count() == 8);
      REQUIRE(operation_log.m_adds.size() == 4);
      REQUIRE(operation_log.m_removes.size() == 4);
    });
  }
}
