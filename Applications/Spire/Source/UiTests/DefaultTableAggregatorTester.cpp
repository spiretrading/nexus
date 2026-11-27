#include <doctest/doctest.h>
#include "Spire/Ui/DefaultTableAggregator.hpp"
#include "Spire/Ui/DistinctTableAggregator.hpp"

using namespace Spire;

TEST_SUITE("DefaultTableAggregator") {
  TEST_CASE("numeric_column") {
    auto aggregator = DefaultTableAggregator();
    aggregator.add(AnyRef());
    REQUIRE_FALSE(aggregator.get().has_value());
    aggregator.update(AnyRef(), 7);
    aggregator.add(5);
    REQUIRE(std::any_cast<int>(aggregator.get()) == 12);
    aggregator.update(7, 3);
    aggregator.remove(5);
    REQUIRE(std::any_cast<int>(aggregator.get()) == 3);
  }

  TEST_CASE("text_column") {
    auto aggregator = DefaultTableAggregator();
    auto eur = QString("EUR");
    auto cad = QString("CAD");
    aggregator.add(eur);
    aggregator.add(eur);
    REQUIRE(std::any_cast<DistinctCount>(aggregator.get()) == DistinctCount(1));
    aggregator.update(eur, cad);
    REQUIRE(std::any_cast<DistinctCount>(aggregator.get()) == DistinctCount(2));
    aggregator.remove(eur);
    aggregator.remove(cad);
    REQUIRE_FALSE(aggregator.get().has_value());
  }
}
