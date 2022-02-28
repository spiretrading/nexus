#include <doctest/doctest.h>
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorTableModel.hpp"
#include "Spire/SpireTester/TableOperationLog.hpp"

using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_imbalance(const auto& symbol, auto timestamp) {
    return OrderImbalance(Security(symbol, DefaultCountries::US()), Side::ASK,
      Quantity(timestamp), Money(timestamp), from_time_t(timestamp));
  }

  auto row_equals(const auto& model, auto row, auto imbalance) {
    return model->get<Security>(row, 0) == imbalance.m_security &&
      model->get<Side>(row, 1) == imbalance.m_side &&
      model->get<Quantity>(row, 2) == imbalance.m_size &&
      model->get<Money>(row, 3) == imbalance.m_referencePrice &&
      model->get<Money>(row, 4) ==
        imbalance.m_size * imbalance.m_referencePrice &&
      model->get<date>(row, 5) == imbalance.m_timestamp.date() &&
      model->get<time_duration>(row, 6) == imbalance.m_timestamp.time_of_day();
  }

  const auto A100 = make_imbalance("A", 100);
  const auto A300 = make_imbalance("A", 300);
  const auto B100 = make_imbalance("B", 100);
  const auto B350 = make_imbalance("B", 350);
}

TEST_SUITE("LocalOrderImbalanceIndicatorTableModel") {
  TEST_CASE("add") {
    auto model = std::make_shared<LocalOrderImbalanceIndicatorTableModel>();
    auto log = TableOperationLog(model);
    model->add(A100);
    REQUIRE(model->get_row_size() == 1);
    REQUIRE(row_equals(model, 0, A100));
    REQUIRE(log.operation_count() == 1);
    REQUIRE(log.add_count() == 1);
    model->add(B100);
    REQUIRE(model->get_row_size() == 2);
    REQUIRE(row_equals(model, 0, A100));
    REQUIRE(row_equals(model, 1, B100));
    REQUIRE(log.operation_count() == 2);
    REQUIRE(log.add_count() == 2);
    model->add(B100);
    REQUIRE(log.operation_count() == 2);
    model->add(B350);
    REQUIRE(model->get_row_size() == 2);
    REQUIRE(row_equals(model, 0, A100));
    REQUIRE(row_equals(model, 1, B350));
    REQUIRE(log.operation_count() == 6);
    REQUIRE(log.add_count() == 2);
    REQUIRE(log.update_count() == 4);
    model->add(A300);
    REQUIRE(model->get_row_size() == 2);
    REQUIRE(row_equals(model, 0, A300));
    REQUIRE(row_equals(model, 1, B350));
    REQUIRE(log.operation_count() == 10);
    REQUIRE(log.add_count() == 2);
    REQUIRE(log.update_count() == 8);
  }

  TEST_CASE("remove") {
    auto model = std::make_shared<LocalOrderImbalanceIndicatorTableModel>();
    auto log = TableOperationLog(model);
    model->add(A100);
    model->add(B350);
    model->remove(A100.m_security);
    REQUIRE(model->get_row_size() == 1);
    REQUIRE(row_equals(model, 0, B350));
    REQUIRE(log.operation_count() == 3);
    REQUIRE(log.add_count() == 2);
    REQUIRE(log.remove_count() == 1);
    model->remove(B350.m_security);
    REQUIRE(model->get_row_size() == 0);
    REQUIRE(log.operation_count() == 4);
    REQUIRE(log.add_count() == 2);
    REQUIRE(log.remove_count() == 2);
  }
}
