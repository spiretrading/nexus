#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServicesTests/ServiceClientFixture.hpp>
#include <doctest/doctest.h>
#include "Nexus/ChartingService/ServiceChartingClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

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
  TEST_CASE("load_time_price_series") {
    auto fixture = Fixture();
    auto security = Security("TST", TSX);
    auto start = from_iso_string("20240131T100000");
    auto end = from_iso_string("20240131T110000");
    auto interval = time_duration(0, 1, 0);
    auto result = TimePriceQueryResult();
    result.start = Beam::Sequence(543);
    result.end = Beam::Sequence(654);
    result.series.push_back(TimePriceCandlestick(
      from_iso_string("20240131T100000"), from_iso_string("20240131T100100"),
      10 * Money::ONE, 11 * Money::ONE, 12 * Money::ONE, 9 * Money::ONE));
    fixture.on_request<LoadSecurityTimePriceSeriesService>(
      [&] (auto& request, const auto& received_security,
          const auto& received_start, const auto& received_end,
          const auto& received_interval) {
        REQUIRE(received_security == security);
        REQUIRE(received_start == start);
        REQUIRE(received_end == end);
        REQUIRE(received_interval == interval);
        request.set(result);
      });
    auto received_result =
      fixture.m_client->load_time_price_series(security, start, end, interval);
    REQUIRE(result.series == received_result.series);
    REQUIRE(result.start == received_result.start);
    REQUIRE(result.end == received_result.end);
  }
}
