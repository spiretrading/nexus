#include <sstream>
#include <string>
#include <vector>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/MarketDataType.hpp"

using namespace Nexus;

TEST_SUITE("MarketDataType") {
  TEST_CASE("stream") {
    auto cases = std::vector<std::pair<MarketDataType, std::string>>{
      {MarketDataType::TIME_AND_SALE, "TIME_AND_SALE"},
      {MarketDataType::BOOK_QUOTE, "BOOK_QUOTE"},
      {MarketDataType::BBO_QUOTE, "BBO_QUOTE"},
      {MarketDataType::ORDER_IMBALANCE, "ORDER_IMBALANCE"},
      {static_cast<MarketDataType>(-1), "NONE"}};
    for(auto& test_case : cases) {
      auto ss = std::ostringstream();
      ss << test_case.first;
      REQUIRE(ss.str() == test_case.second);
    }
  }

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
