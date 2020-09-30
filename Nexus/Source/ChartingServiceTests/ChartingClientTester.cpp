#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/ChartingService/ChartingClient.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::ChartingService;
using namespace Nexus::TechnicalAnalysis;

namespace {
  struct Fixture {
    using TestChartingClient = ChartingClient<TestServiceProtocolClientBuilder>;

    boost::optional<TestServiceProtocolServer> m_server;
    boost::optional<TestChartingClient> m_client;

    Fixture() {
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_server.emplace(serverConnection,
        factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
      RegisterChartingServices(Store(m_server->GetSlots()));
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
            "test", *serverConnection);
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_client.emplace(builder);
    }
  };
}

TEST_SUITE("ChartingClient") {
  TEST_CASE_FIXTURE(Fixture, "load_security_price_time_series") {
    auto security = Security("TST", DefaultMarkets::NYSE(),
      DefaultCountries::US());
    auto startTime = ptime(date(2010, May, 6));
    auto endTime = ptime(date(2011, May, 6));
    auto interval = hours(24);
    auto receivedRequest = false;
    auto expectedSeries = TimePriceSeries();
    expectedSeries.emplace_back(startTime, startTime + days(1), Money::ONE,
      Money::ONE, Money::ONE, Money::ONE);
    expectedSeries.emplace_back(startTime + days(1), startTime + days(2),
      2 * Money::ONE, 2 * Money::ONE, 2 * Money::ONE, 2 * Money::ONE);
    LoadSecurityTimePriceSeriesService::AddSlot(Store(m_server->GetSlots()),
      [&] (auto& client, const Security& serviceSecurity,
          ptime serviceStartTime, ptime serviceEndTime,
          time_duration serviceInterval) {
        receivedRequest = true;
        REQUIRE(serviceSecurity == security);
        REQUIRE(serviceStartTime == startTime);
        REQUIRE(serviceEndTime == endTime);
        REQUIRE(serviceInterval == interval);
        auto result = TimePriceQueryResult();
        result.series = expectedSeries;
        return result;
      });
    auto series = m_client->LoadTimePriceSeries(security, startTime, endTime,
      interval);
    REQUIRE(receivedRequest);
    REQUIRE(series.series == expectedSeries);
  }
}
