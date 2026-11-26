#include <cmath>
#include <limits>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include <QString>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Ui/Details/DefaultTableAggregation.hpp"
#include "Spire/Ui/TableAggregation.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

TEST_SUITE("TableAggregation") {
  TEST_CASE("integer_sum") {
    auto aggregate = make_default_table_aggregation();
    REQUIRE_FALSE(aggregate->get().has_value());
    auto large = std::int64_t(1) << 54;
    aggregate->add(large);
    aggregate->add(5U);
    aggregate->add(-2);
    auto previous = aggregate->get();
    aggregate->update(large, 3);
    REQUIRE(std::any_cast<std::int64_t>(previous) == large + 3);
    REQUIRE(std::any_cast<std::int64_t>(aggregate->get()) == 6);
    aggregate->remove(5U);
    REQUIRE(std::any_cast<std::int64_t>(aggregate->get()) == 1);
  }

  TEST_CASE("real_sum") {
    auto aggregate = make_default_table_aggregation();
    auto maximum = std::numeric_limits<double>::max();
    aggregate->add(maximum);
    aggregate->add(maximum);
    REQUIRE(std::isinf(std::any_cast<double>(aggregate->get())));
    aggregate->remove(maximum);
    REQUIRE(std::any_cast<double>(aggregate->get()) == maximum);
    aggregate->update(maximum, 0.25f);
    aggregate->add(1);
    REQUIRE(std::any_cast<double>(aggregate->get()) == 1.25);
    auto large = std::int64_t(1) << 60;
    aggregate->update(0.25f, static_cast<double>(large));
    aggregate->update(1, 1 - large);
    REQUIRE(std::any_cast<double>(aggregate->get()) == 1.0);
  }

  TEST_CASE("real_sum_after_cancellation") {
    auto aggregate = make_default_table_aggregation();
    auto large = std::ldexp(1.0, 240);
    auto intermediate = std::ldexp(1.0, 120);
    aggregate->add(large);
    aggregate->add(intermediate);
    aggregate->add(1.0);
    aggregate->remove(large);
    aggregate->remove(intermediate);
    REQUIRE(std::any_cast<double>(aggregate->get()) == 1.0);
    auto smallest = std::numeric_limits<double>::denorm_min();
    auto maximum = std::numeric_limits<double>::max();
    aggregate->update(1.0, -smallest);
    aggregate->add(-maximum);
    aggregate->add(maximum);
    REQUIRE(std::any_cast<double>(aggregate->get()) == -smallest);
    aggregate->remove(maximum);
    aggregate->remove(-maximum);
    REQUIRE(std::any_cast<double>(aggregate->get()) == -smallest);
  }

  TEST_CASE("money_and_quantity_sums") {
    auto aggregate = make_default_table_aggregation();
    auto cent = Money::CENT;
    aggregate->add(optional<Money>());
    REQUIRE_FALSE(aggregate->get().has_value());
    aggregate->add(cent);
    aggregate->add(make_optional(cent));
    REQUIRE(std::any_cast<Money>(aggregate->get()) == cent + cent);
    aggregate->remove(make_optional(cent));
    aggregate->update(cent, Quantity(0.5));
    aggregate->add(Quantity(0.25));
    REQUIRE(std::any_cast<Quantity>(aggregate->get()) == Quantity(0.75));
  }

  TEST_CASE("distinct_text_count") {
    auto factory = TableAggregationFactory(&make_default_table_aggregation);
    auto first = factory();
    auto second = factory();
    first->add(optional<QString>());
    REQUIRE_FALSE(first->get().has_value());
    first->add(QString("CAD"));
    auto duplicate = boost::make_optional(std::string("CAD"));
    first->add(duplicate);
    first->add(QString("USD"));
    REQUIRE(std::any_cast<QString>(first->get()) == "(2)");
    first->remove(QString("CAD"));
    REQUIRE(std::any_cast<QString>(first->get()) == "(2)");
    first->update(duplicate, QString("USD"));
    REQUIRE(std::any_cast<QString>(first->get()) == "(1)");
    REQUIRE_FALSE(second->get().has_value());
    second->add(QString("EUR"));
    first->remove(QString("USD"));
    first->remove(QString("USD"));
    REQUIRE_FALSE(first->get().has_value());
    REQUIRE(std::any_cast<QString>(second->get()) == "(1)");
  }

  TEST_CASE("optional_side_count") {
    auto aggregate = make_default_table_aggregation();
    auto missing = optional<Side>();
    auto bid = optional<Side>(Side::BID);
    auto ask = optional<Side>(Side::ASK);
    SUBCASE("empty") {
      aggregate->add(missing);
      REQUIRE_FALSE(aggregate->get().has_value());
      aggregate->update(missing, bid);
      REQUIRE(std::any_cast<QString>(aggregate->get()) == "(1)");
      aggregate->update(bid, missing);
      REQUIRE_FALSE(aggregate->get().has_value());
    }
    SUBCASE("distinct_values") {
      aggregate->add(bid);
      aggregate->add(bid);
      aggregate->add(ask);
      REQUIRE(std::any_cast<QString>(aggregate->get()) == "(2)");
      aggregate->remove(bid);
      REQUIRE(std::any_cast<QString>(aggregate->get()) == "(2)");
      aggregate->update(ask, bid);
      REQUIRE(std::any_cast<QString>(aggregate->get()) == "(1)");
    }
  }

  TEST_CASE("timestamp_distinct_count") {
    auto midnight = time_from_string("2026-11-25 00:00:00");
    auto previous = midnight - seconds(1);
    auto values = std::vector<std::any>{previous, midnight, previous, 7};
    auto aggregate = std::unique_ptr<TableAggregation>();
    SUBCASE("standalone") {
      aggregate = make_default_table_aggregation();
    }
    SUBCASE("group") {
      aggregate = Spire::Details::make_default_table_aggregation(
        [&] (const Spire::Details::TableValueVisitor& visit) {
          for(auto& value : values) {
            visit(value);
          }
        });
    }
    for(auto& value : values) {
      aggregate->add(value);
    }
    REQUIRE(std::any_cast<QString>(aggregate->get()) == "(3)");
    values[0] = QString("2026-11-24T23:59:59");
    aggregate->update(previous, values[0]);
    REQUIRE(std::any_cast<QString>(aggregate->get()) == "(3)");
    aggregate->remove(values[2]);
    REQUIRE(std::any_cast<QString>(aggregate->get()) == "(3)");
    aggregate->remove(values[0]);
    REQUIRE(std::any_cast<QString>(aggregate->get()) == "(2)");
    aggregate->remove(midnight);
    REQUIRE(std::any_cast<std::int64_t>(aggregate->get()) == 7);
  }

  TEST_CASE("mixed_values") {
    auto values = std::vector<std::any>{3, -2, optional<int>()};
    auto visits = 0;
    auto expected_visits = 0;
    auto aggregate = std::unique_ptr<TableAggregation>();
    SUBCASE("standalone") {
      aggregate = make_default_table_aggregation();
    }
    SUBCASE("group") {
      expected_visits = 1;
      aggregate = Spire::Details::make_default_table_aggregation(
        [&] (const Spire::Details::TableValueVisitor& visit) {
          ++visits;
          for(auto& value : values) {
            visit(value);
          }
        });
    }
    for(auto& value : values) {
      aggregate->add(value);
    }
    REQUIRE(std::any_cast<std::int64_t>(aggregate->get()) == 1);
    REQUIRE(visits == 0);
    values.emplace_back(QString("3"));
    aggregate->add(values.back());
    REQUIRE(std::any_cast<QString>(aggregate->get()) == "(2)");
    REQUIRE(visits == expected_visits);
    values[1] = make_optional(3);
    aggregate->update(-2, values[1]);
    REQUIRE(std::any_cast<QString>(aggregate->get()) == "(1)");
    aggregate->remove(values.back());
    values.pop_back();
    REQUIRE(std::any_cast<std::int64_t>(aggregate->get()) == 6);
    REQUIRE(visits == expected_visits);
  }
}
