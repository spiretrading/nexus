#include <cmath>
#include <limits>
#include <doctest/doctest.h>
#include "Spire/Ui/SumTableAggregator.hpp"

using namespace Nexus;
using namespace Spire;

TEST_SUITE("SumTableAggregator") {
  TEST_CASE("integer_updates") {
    auto aggregate = SumTableAggregator<int>();
    auto missing = AnyRef();
    aggregate.add(missing);
    REQUIRE_FALSE(aggregate.get().has_value());
    aggregate.add(5);
    aggregate.add(5);
    aggregate.update(missing, 7);
    aggregate.remove(5);
    REQUIRE(std::any_cast<std::int64_t>(aggregate.get()) == 12);
    aggregate.update(7, missing);
    aggregate.remove(5);
    REQUIRE_FALSE(aggregate.get().has_value());
  }

  TEST_CASE("integer_sum") {
    auto aggregate = SumTableAggregator<std::int64_t>();
    REQUIRE_FALSE(aggregate.get().has_value());
    auto large = std::int64_t(1) << 54;
    aggregate.add(large);
    aggregate.add(std::int64_t(5));
    aggregate.add(std::int64_t(-2));
    auto previous = aggregate.get();
    aggregate.update(large, std::int64_t(3));
    REQUIRE(std::any_cast<std::int64_t>(previous) == large + 3);
    REQUIRE(std::any_cast<std::int64_t>(aggregate.get()) == 6);
    aggregate.remove(std::int64_t(5));
    REQUIRE(std::any_cast<std::int64_t>(aggregate.get()) == 1);
  }

  TEST_CASE("real_sum") {
    auto aggregate = SumTableAggregator<double>();
    auto maximum = std::numeric_limits<double>::max();
    aggregate.add(maximum);
    aggregate.add(maximum);
    REQUIRE(std::isinf(std::any_cast<double>(aggregate.get())));
    aggregate.remove(maximum);
    REQUIRE(std::any_cast<double>(aggregate.get()) == maximum);
    aggregate.update(maximum, 0.25);
    aggregate.add(1.0);
    REQUIRE(std::any_cast<double>(aggregate.get()) == 1.25);
  }

  TEST_CASE("real_sum_after_cancellation") {
    auto aggregate = SumTableAggregator<double>();
    auto large = std::ldexp(1.0, 240);
    auto intermediate = std::ldexp(1.0, 120);
    aggregate.add(large);
    aggregate.add(intermediate);
    aggregate.add(1.0);
    aggregate.remove(large);
    aggregate.remove(intermediate);
    REQUIRE(std::any_cast<double>(aggregate.get()) == 1.0);
    auto smallest = std::numeric_limits<double>::denorm_min();
    auto maximum = std::numeric_limits<double>::max();
    aggregate.update(1.0, -smallest);
    aggregate.add(-maximum);
    aggregate.add(maximum);
    REQUIRE(std::any_cast<double>(aggregate.get()) == -smallest);
    aggregate.remove(maximum);
    aggregate.remove(-maximum);
    REQUIRE(std::any_cast<double>(aggregate.get()) == -smallest);
  }

  TEST_CASE("money_sum") {
    auto aggregate = SumTableAggregator<Money>();
    auto cent = Money::CENT;
    auto missing = AnyRef();
    aggregate.add(missing);
    REQUIRE_FALSE(aggregate.get().has_value());
    aggregate.add(cent);
    aggregate.add(cent);
    REQUIRE(std::any_cast<Money>(aggregate.get()) == cent + cent);
    aggregate.remove(cent);
    REQUIRE(std::any_cast<Money>(aggregate.get()) == cent);
    aggregate.add(-cent);
    REQUIRE(std::any_cast<Money>(aggregate.get()) == Money::ZERO);
    aggregate.remove(cent);
    aggregate.update(-cent, missing);
    REQUIRE_FALSE(aggregate.get().has_value());
    aggregate.update(missing, cent);
    REQUIRE(std::any_cast<Money>(aggregate.get()) == cent);
  }

  TEST_CASE("quantity_sum") {
    auto aggregate = SumTableAggregator<Quantity>();
    aggregate.add(Quantity(0.5));
    aggregate.add(Quantity(0.25));
    REQUIRE(std::any_cast<Quantity>(aggregate.get()) == Quantity(0.75));
  }
}
