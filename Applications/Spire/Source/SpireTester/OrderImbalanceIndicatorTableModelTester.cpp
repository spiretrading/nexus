#include <doctest/doctest.h>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorTableModel.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  struct TableOperationLog {
    std::vector<TableModel::AddOperation> m_adds;
    std::vector<TableModel::RemoveOperation> m_removes;
    std::vector<TableModel::MoveOperation> m_moves;
    std::vector<TableModel::UpdateOperation> m_updates;

    bool counts_equal(std::size_t add_count, std::size_t remove_count,
        std::size_t move_count, std::size_t update_count) {
      return m_adds.size() == add_count && m_removes.size() == remove_count &&
        m_moves.size() == move_count && m_updates.size() == update_count;
    }

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

  // TODO: maybe add these to a header instead of copy-pasting from the local
  //       imbalance tests.
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

  auto load_closed(auto& model, auto lower, auto upper) {
    auto result = model.subscribe(closed(lower, upper), [] (const auto&) {});
    return wait(std::move(result.m_snapshot));
  }

  auto load_open(auto& model, auto lower, auto upper) {
    auto result = model.subscribe(open(lower, upper), [] (const auto&) {});
    return wait(std::move(result.m_snapshot));
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

  auto row_equals(const auto& model, auto index, const auto& imbalance) {
    return model.get<Security>(index, 0) == imbalance.m_security &&
      model.get<Side>(index, 1) == imbalance.m_side &&
      model.get<Quantity>(index, 2) == imbalance.m_size &&
      model.get<Money>(index, 3) == imbalance.m_referencePrice &&
      model.get<Money>(index, 4) ==
        imbalance.m_size * imbalance.m_referencePrice &&
      ptime(model.get<boost::gregorian::date>(index, 5),
        model.get<time_duration>(index, 6)) == imbalance.m_timestamp;
  }

  const auto A100 = make_imbalance("A", 100);
  const auto B100 = make_imbalance("B", 100);
}

TEST_SUITE("OrderImbalanceIndicatorTableModel") {
  TEST_CASE("set_interval") {
    run_test([] {
      auto test_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      auto model = OrderImbalanceIndicatorTableModel(test_model);
      test_model->publish(A100);
      test_model->publish(B100);
      auto operation_log = TableOperationLog{};
      model.connect_operation_signal(
        std::bind_front(&TableOperationLog::on_operation, &operation_log));
      model.set_interval(closed(0, 200));
      wait_until([&] { return operation_log.operation_count() == 2; });
      REQUIRE(model.get_row_size() == 2);
      REQUIRE(row_equals(model, 0, A100));
      REQUIRE(row_equals(model, 1, B100));
      REQUIRE(operation_log.counts_equal(2, 0, 0, 0));
      model.set_interval(open(0, 100));
      wait_until([&] { return operation_log.operation_count() == 4; });
      REQUIRE(model.get_row_size() == 0);
      // TODO: should these deletions cause some intermediate move operations?
      REQUIRE(operation_log.counts_equal(2, 2, 0, 0));
    });
  }

  TEST_CASE("set_offset") {
    
  }

  TEST_CASE("published_imbalances") {
    
  }

  TEST_CASE("expired_imbalances") {
    
  }
}
