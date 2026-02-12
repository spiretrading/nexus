#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServicesTests/ServiceClientFixture.hpp>
#include <doctest/doctest.h>
#include "Nexus/ChartingService/ServiceChartingClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct Fixture : ServiceClientFixture {
    using TestServiceChartingClient =
      ServiceChartingClient<TestServiceProtocolClientBuilder>;
    std::unique_ptr<TestServiceChartingClient> m_client;

    Fixture() {
      register_charting_services(out(m_server.get_slots()));
      register_charting_messages(out(m_server.get_slots()));
      m_client = make_client<TestServiceChartingClient>();
    }
  };
}

TEST_SUITE("ServiceChartingClient") {
  TEST_CASE("load_price_series") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto start = time_from_string("2024-01-31 10:00:00");
    auto end = time_from_string("2024-01-31 11:00:00");
    auto interval = duration_from_string("00:01:00");
    auto result = PriceQueryResult();
    result.start = Beam::Sequence(543);
    result.end = Beam::Sequence(654);
    result.series.push_back(PriceCandlestick(
      time_from_string("2024-01-31 10:00:00"),
      time_from_string("2024-01-31 10:01:00"),
      10 * Money::ONE, 11 * Money::ONE, 12 * Money::ONE, 9 * Money::ONE, 200));
    fixture.on_request<LoadTickerPriceSeriesService>(
      [&] (auto& request, const auto& received_ticker,
          const auto& received_start, const auto& received_end,
          const auto& received_interval) {
        REQUIRE(received_ticker == ticker);
        REQUIRE(received_start == start);
        REQUIRE(received_end == end);
        REQUIRE(received_interval == interval);
        request.set(result);
      });
    auto received_result =
      fixture.m_client->load_price_series(ticker, start, end, interval);
    REQUIRE(result.series == received_result.series);
    REQUIRE(result.start == received_result.start);
    REQUIRE(result.end == received_result.end);
  }
}
