#include "Nexus/ChartingServiceTests/ChartingClientTester.hpp"
#include <Beam/ServiceLocator/NullAuthenticator.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus::ChartingService;
using namespace Nexus::ChartingService::Tests;
using namespace Nexus::TechnicalAnalysis;
using namespace std;

void ChartingClientTester::setUp() {
  m_serverConnection.Initialize();
  m_server.Initialize(&*m_serverConnection,
    factory<std::shared_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
  RegisterChartingServices(Store(m_server->GetSlots()));
  ServiceProtocolClientBuilder builder(
    [&] {
      return std::make_unique<ServiceProtocolClientBuilder::Channel>(("test"),
        Ref(*m_serverConnection));
    },
    [&] {
      return std::make_unique<ServiceProtocolClientBuilder::Timer>();
    });
  m_client.Initialize(builder);
  m_server->Open();
  m_client->Open();
}

void ChartingClientTester::tearDown() {
  m_client.Reset();
  m_server.Reset();
  m_serverConnection.Reset();
}

void ChartingClientTester::TestLoadSecurityPriceTimeSeries() {
  Security security("TST", DefaultMarkets::NYSE(), DefaultCountries::US());
  ptime startTime(date(2010, May, 6));
  ptime endTime(date(2011, May, 6));
  time_duration interval = hours(24);
  bool receivedRequest = false;
  TimePriceSeries expectedSeries;
  expectedSeries.emplace_back(startTime, startTime + days(1), Money::ONE,
    Money::ONE, Money::ONE, Money::ONE);
  expectedSeries.emplace_back(startTime + days(1), startTime + days(2),
    2 * Money::ONE, 2 * Money::ONE, 2 * Money::ONE, 2 * Money::ONE);
  LoadSecurityTimePriceSeriesService::AddSlot(Store(m_server->GetSlots()),
    [&] (ServiceProtocolServer::ServiceProtocolClient& client,
        const Security& serviceSecurity, const ptime& serviceStartTime,
        const ptime& serviceEndTime,
        const time_duration& serviceInterval) -> TimePriceQueryResult {
      receivedRequest = true;
      CPPUNIT_ASSERT(serviceSecurity == security);
      CPPUNIT_ASSERT(serviceStartTime == startTime);
      CPPUNIT_ASSERT(serviceEndTime == endTime);
      CPPUNIT_ASSERT(serviceInterval == interval);
      TimePriceQueryResult result;
      result.series = expectedSeries;
      return result;
    });
  TimePriceQueryResult series = m_client->LoadTimePriceSeries(security,
    startTime, endTime, interval);
  CPPUNIT_ASSERT(receivedRequest);
  CPPUNIT_ASSERT(series.series == expectedSeries);
}
