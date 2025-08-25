#include <future>
#include <Beam/Queues/Queue.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"
#include "Nexus/TechnicalAnalysis/StandardSecurityQueries.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

TEST_SUITE("StandardSecurityQueries") {
  TEST_CASE("load_open") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto security = Security("TST", TSX);
    auto date = time_from_string("2024-07-16 00:00:00");
    auto open_time_and_sale =
      TimeAndSale(time_from_string("2024-07-16 13:30:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "T");
    auto result = std::async(std::launch::async, [&] {
      return load_open(client, security, date, DEFAULT_VENUES,
        get_default_time_zone_database());
    });
    auto operation = operations->Pop();
    auto& time_and_sale_operation =
      std::get<TestMarketDataClient::QueryTimeAndSaleOperation>(*operation);
    time_and_sale_operation.m_queue.Push(open_time_and_sale);
    auto open = result.get();
    REQUIRE(open == open_time_and_sale);
  }

  TEST_CASE("make_daily_query_range") {
    auto security = Security("TST", TSX);
    auto start_date = ptime(date(2024, 7, 16));
    auto end_date = ptime(date(2024, 7, 16));
    auto range = make_daily_query_range(security, start_date, end_date,
      DEFAULT_VENUES, get_default_time_zone_database());
    REQUIRE(range.GetStart() == time_from_string("2024-07-16 04:00:00"));
    REQUIRE(range.GetEnd() == time_from_string("2024-07-17 04:00:00"));
  }

  TEST_CASE("make_query") {
    auto security = Security("TST", TSX);
    auto start_date = ptime(date(2024, 7, 16));
    auto end_date = ptime(date(2024, 7, 16));
    SUBCASE("time_and_sale") {
      auto min = MakeMinExpression(ParameterExpression(0, MoneyType()),
        ParameterExpression(1, MoneyType()));
      auto low = ReduceExpression(min, TimeAndSaleAccessor(
        ParameterExpression(0, TimeAndSaleType())).get_price(),
        MoneyValue(99999999 * Money::ONE));
      auto query = make_query(security, start_date, end_date, DEFAULT_VENUES,
        get_default_time_zone_database(), low);
      REQUIRE(query.GetIndex() == security);
      REQUIRE(query.get_market_data_type() == MarketDataType::TIME_AND_SALE);
      REQUIRE(
        query.GetRange().GetStart() == time_from_string("2024-07-16 04:00:00"));
      REQUIRE(
        query.GetRange().GetEnd() == time_from_string("2024-07-17 04:00:00"));
    }
  }
}
