#include <sstream>
#include <string>
#include <vector>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/MarketDataType.hpp"

using namespace Nexus;

TEST_SUITE("MarketDataType") {
  TEST_CASE("get_market_data_type") {
    REQUIRE(
      get_market_data_type<TimeAndSale>() == MarketDataType::TIME_AND_SALE);
    REQUIRE(get_market_data_type<BookQuote>() == MarketDataType::BOOK_QUOTE);
    REQUIRE(get_market_data_type<BboQuote>() == MarketDataType::BBO_QUOTE);
    REQUIRE(get_market_data_type<OrderImbalance>() ==
      MarketDataType::ORDER_IMBALANCE);
    REQUIRE(get_market_data_type<std::string>() == MarketDataType::NONE);
  }
}
