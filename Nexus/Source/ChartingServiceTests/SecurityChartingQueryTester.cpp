#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("SecurityChartingQuery") {
  TEST_CASE("get_set_market_data_type") {
    auto query = SecurityChartingQuery();
    query.set_market_data_type(MarketDataType::BBO_QUOTE);
    REQUIRE(query.get_market_data_type() == MarketDataType::BBO_QUOTE);
    query.set_market_data_type(MarketDataType::TIME_AND_SALE);
    REQUIRE(query.get_market_data_type() == MarketDataType::TIME_AND_SALE);
  }
}
