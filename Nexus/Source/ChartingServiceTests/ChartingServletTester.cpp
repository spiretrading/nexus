#include "Nexus/ChartingServiceTests/ChartingServletTester.hpp"
#include <boost/functional/factory.hpp>

using namespace Beam;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::ChartingService;
using namespace Nexus::ChartingService::Tests;
using namespace Nexus::TechnicalAnalysis;
using namespace std;

void ChartingServletTester::setUp() {
  m_environment.emplace();
  m_environment->Open();
  m_serviceClients.emplace(Ref(*m_environment));
  m_serviceClients->Open();
  m_serverConnection = std::make_shared<ServerConnection>();
  m_clientProtocol.emplace(Initialize(string("test"),
    Ref(*m_serverConnection)), Initialize());
  RegisterChartingServices(Store(m_clientProtocol->GetSlots()));
  m_container.emplace(Initialize(&m_serviceClients->GetMarketDataClient()),
    m_serverConnection, factory<std::shared_ptr<TriggerTimer>>());
  m_container->Open();
  m_clientProtocol->Open();
}

void ChartingServletTester::tearDown() {
  m_clientProtocol.reset();
  m_container.reset();
  m_serverConnection.reset();
  m_serviceClients.reset();
  m_environment.reset();
}

void ChartingServletTester::TestLoadSecurityTimePriceSeries() {
  Security security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
  ptime startTime{date{2010, May, 6}, time_duration{5, 0, 0, 0}};
  auto endTime = startTime + minutes(5);
  time_duration interval = minutes(1);
  TimePriceSeries expectedSeries;
  for(int i = 0; i < 5; ++i) {
    auto timestamp = startTime + minutes(i);
    auto price = (i + 1) * Money::ONE;
    SecurityTimeAndSale timeAndSale{TimeAndSale{timestamp, price, i * 100,
      TimeAndSale::Condition{TimeAndSale::Condition::Type::NONE, "?"}, "N"},
      security};
    m_environment->GetMarketDataEnvironment().GetDataStore().Store(
      MakeSequencedValue(timeAndSale, Beam::Queries::Sequence(i + 1)));
    expectedSeries.emplace_back(timestamp, timestamp + interval, price, price,
      price, price);
  }
  auto result = m_clientProtocol->SendRequest<
    LoadSecurityTimePriceSeriesService>(security, startTime, endTime,
    interval);
  CPPUNIT_ASSERT(result.series == expectedSeries);
}
