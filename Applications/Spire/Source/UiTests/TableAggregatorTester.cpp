#include <doctest/doctest.h>
#include "Spire/Ui/TableAggregator.hpp"

using namespace Spire;

TEST_SUITE("TableAggregator") {
  TEST_CASE("default_factory") {
    auto factory = TableAggregatorFactory(&make_default_table_aggregator);
    auto first = factory();
    auto second = factory();
    first->add(7);
    REQUIRE_FALSE(second->get().has_value());
    second->add(5);
    first->remove(7);
    REQUIRE_FALSE(first->get().has_value());
    REQUIRE(std::any_cast<std::int64_t>(second->get()) == 5);
  }
}
