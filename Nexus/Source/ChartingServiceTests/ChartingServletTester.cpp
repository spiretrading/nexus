#include <Beam/ServicesTests/TestServices.hpp>
#include <boost/optional/optional.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/ChartingService/ChartingServlet.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::ChartingService;
using namespace Nexus::MarketDataService;
using namespace Nexus::TechnicalAnalysis;

namespace {
  struct Fixture {
    using ServletContainer = TestServiceProtocolServletContainer<
      MetaChartingServlet<MarketDataClientBox>>;

    TestEnvironment m_environment;
    TestServiceClients m_serviceClients;
    boost::optional<ServletContainer> m_container;
    boost::optional<Beam::Services::Tests::TestServiceProtocolClient>
      m_protocolClient;

    Fixture()
        : m_serviceClients(Ref(m_environment)) {
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_container.emplace(Initialize(m_serviceClients.GetMarketDataClient()),
        serverConnection, factory<std::unique_ptr<TriggerTimer>>());
      m_protocolClient.emplace(Initialize("test", *serverConnection),
        Initialize());
      RegisterChartingServices(Store(m_protocolClient->GetSlots()));
    }
  };
}

TEST_SUITE("ChartingServlet") {
  TEST_CASE_FIXTURE(Fixture, "load_security_time_price_series") {
    auto security = Security("TST", DefaultMarkets::NYSE(),
      DefaultCountries::US());
    auto startTime = ptime(date(2010, May, 6), time_duration(5, 0, 0, 0));
    auto endTime = startTime + minutes(5);
    auto interval = minutes(1);
    auto expectedSeries = TimePriceSeries();
    for(int i = 0; i < 5; ++i) {
      auto timestamp = startTime + minutes(i);
      auto price = (i + 1) * Money::ONE;
      auto timeAndSale = TimeAndSale(timestamp, price, i * 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::NONE, "?"), "N",
        "A", "B");
      m_environment.GetMarketDataEnvironment().GetFeedClient().Publish(
        SecurityTimeAndSale(timeAndSale, security));
      expectedSeries.emplace_back(timestamp, timestamp + interval, price, price,
        price, price);
    }
    auto result =
      m_protocolClient->SendRequest<LoadSecurityTimePriceSeriesService>(
        security, startTime, endTime, interval);
    REQUIRE(result.series == expectedSeries);
  }
}
