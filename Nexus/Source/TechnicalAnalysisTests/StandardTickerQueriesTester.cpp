#include <future>
#include <Beam/Queues/Queue.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"
#include "Nexus/TechnicalAnalysis/StandardTickerQueries.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;

TEST_SUITE("StandardTickerQueries") {
  TEST_CASE("load_open") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto ticker = parse_ticker("TST.TSX");
    auto date = time_from_string("2024-07-16 00:00:00");
    auto open_time_and_sale =
      TimeAndSale(time_from_string("2024-07-16 13:30:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "T");
    auto result = std::async(std::launch::async, [&] {
      return load_open(client, ticker, date, DEFAULT_VENUES,
        get_default_time_zone_database());
    });
    auto operation = operations->pop();
    auto& time_and_sale_operation =
      std::get<TestMarketDataClient::QueryTimeAndSaleOperation>(*operation);
    time_and_sale_operation.m_queue.push(open_time_and_sale);
    auto open = result.get();
    REQUIRE(open == open_time_and_sale);
  }

  TEST_CASE("make_daily_query_range") {
    auto ticker = parse_ticker("TST.TSX");
    auto start_date = time_from_string("2024-07-16 00:00:00");
    auto end_date = time_from_string("2024-07-16 00:00:00");
    auto range = make_daily_query_range(ticker, start_date, end_date,
      DEFAULT_VENUES, get_default_time_zone_database());
    REQUIRE(range.get_start() == time_from_string("2024-07-15 04:00:00"));
    REQUIRE(range.get_end() == time_from_string("2024-07-16 04:00:00"));
  }

  TEST_CASE("make_query") {
    auto ticker = parse_ticker("TST.TSX");
    auto start_date = time_from_string("2024-07-16 00:00:00");
    auto end_date = time_from_string("2024-07-16 00:00:00");
    SUBCASE("time_and_sale") {
      auto low = ReduceExpression(min(ParameterExpression(0, typeid(Money)),
        ParameterExpression(1, typeid(Money))),
        TimeAndSaleAccessor::from_parameter(0).get_price(),
        99999999 * Money::ONE);
      auto query = make_query(ticker, start_date, end_date, DEFAULT_VENUES,
        get_default_time_zone_database(), low);
      REQUIRE(query.get_index() == ticker);
      REQUIRE(query.get_market_data_type() == MarketDataType::TIME_AND_SALE);
      REQUIRE(query.get_range().get_start() ==
        time_from_string("2024-07-15 04:00:00"));
      REQUIRE(query.get_range().get_end() ==
        time_from_string("2024-07-16 04:00:00"));
    }
  }
}
