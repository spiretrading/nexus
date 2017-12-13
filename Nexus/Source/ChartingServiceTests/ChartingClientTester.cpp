#include "Nexus/ChartingServiceTests/ChartingClientTester.hpp"
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/functional/factory.hpp>
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
using namespace Nexus::ChartingService;
using namespace Nexus::ChartingService::Tests;
using namespace Nexus::TechnicalAnalysis;
using namespace std;

void ChartingClientTester::setUp() {
  auto serverConnection = std::make_shared<TestServerConnection>();
  m_server.emplace(serverConnection, factory<std::unique_ptr<TriggerTimer>>(),
    NullSlot(), NullSlot());
  RegisterChartingServices(Store(m_server->GetSlots()));
  m_server->Open();
  TestServiceProtocolClientBuilder builder{
    [=] {
      return std::make_unique<TestServiceProtocolClientBuilder::Channel>("test",
        Ref(*serverConnection));
    }, factory<unique_ptr<TestServiceProtocolClientBuilder::Timer>>()};
  m_client.emplace(builder);
  m_client->Open();
}

void ChartingClientTester::tearDown() {
  m_client.reset();
  m_server.reset();
}

void ChartingClientTester::TestLoadSecurityPriceTimeSeries() {
  Security security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
  ptime startTime{date{2010, May, 6}};
  ptime endTime{date{2011, May, 6}};
  time_duration interval = hours(24);
  auto receivedRequest = false;
  TimePriceSeries expectedSeries;
  expectedSeries.emplace_back(startTime, startTime + days(1), Money::ONE,
    Money::ONE, Money::ONE, Money::ONE);
  expectedSeries.emplace_back(startTime + days(1), startTime + days(2),
    2 * Money::ONE, 2 * Money::ONE, 2 * Money::ONE, 2 * Money::ONE);
  LoadSecurityTimePriceSeriesService::AddSlot(Store(m_server->GetSlots()),
    [&] (auto& client, const Security& serviceSecurity,
        const ptime& serviceStartTime, const ptime& serviceEndTime,
        const time_duration& serviceInterval) {
      receivedRequest = true;
      CPPUNIT_ASSERT(serviceSecurity == security);
      CPPUNIT_ASSERT(serviceStartTime == startTime);
      CPPUNIT_ASSERT(serviceEndTime == endTime);
      CPPUNIT_ASSERT(serviceInterval == interval);
      TimePriceQueryResult result;
      result.series = expectedSeries;
      return result;
    });
  auto series = m_client->LoadTimePriceSeries(security, startTime, endTime,
    interval);
  CPPUNIT_ASSERT(receivedRequest);
  CPPUNIT_ASSERT(series.series == expectedSeries);
}
