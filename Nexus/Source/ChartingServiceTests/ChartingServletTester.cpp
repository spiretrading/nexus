#include "Nexus/ChartingServiceTests/ChartingServletTester.hpp"
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::ChartingService;
using namespace Nexus::ChartingService::Tests;
using namespace Nexus::MarketDataService;
using namespace Nexus::Queries;
using namespace Nexus::TechnicalAnalysis;
using namespace std;

void ChartingServletTester::setUp() {
  m_serviceLocatorInstance.Initialize();
  m_serviceLocatorInstance->Open();
  m_serviceLocatorClient = m_serviceLocatorInstance->BuildClient();
  m_serviceLocatorClient->SetCredentials("root", "");
  m_serviceLocatorClient->Open();
  std::unique_ptr<ServiceLocatorTestInstance::ServiceLocatorClient>
    marketDataServiceLocatorClient = m_serviceLocatorInstance->BuildClient();
  marketDataServiceLocatorClient->SetCredentials("root", "");
  marketDataServiceLocatorClient->Open();
  m_marketDataServiceInstance.Initialize(
    std::move(marketDataServiceLocatorClient));
  m_marketDataServiceInstance->Open();
  m_serverConnection = std::make_shared<ServerConnection>();
  m_clientProtocol.Initialize(Initialize(string("test"),
    Ref(*m_serverConnection)), Initialize());
  RegisterChartingServices(Store(m_clientProtocol->GetSlots()));
  m_container.Initialize(Initialize(m_marketDataServiceInstance->BuildClient(
    Ref(*m_serviceLocatorClient))), m_serverConnection,
    factory<std::shared_ptr<TriggerTimer>>());
  m_container->Open();
  m_clientProtocol->Open();
}

void ChartingServletTester::tearDown() {
  m_clientProtocol.Reset();
  m_container.Reset();
  m_serverConnection.reset();
  m_marketDataServiceInstance.Reset();
  m_serviceLocatorClient.reset();
  m_serviceLocatorInstance.Reset();
}

void ChartingServletTester::TestLoadSecurityTimePriceSeries() {
  Security security("TST", DefaultMarkets::NYSE(), DefaultCountries::US());
  ptime startTime(date(2010, May, 6), time_duration(5, 0, 0, 0));
  ptime endTime = startTime + minutes(5);
  time_duration interval = minutes(1);
  TimePriceSeries expectedSeries;
  for(int i = 0; i < 5; ++i) {
    ptime timestamp = startTime + minutes(i);
    Money price = (i + 1) * Money::ONE;
    SecurityTimeAndSale timeAndSale(TimeAndSale(timestamp, price, i * 100,
      TimeAndSale::Condition(TimeAndSale::Condition::Type::NONE, "?"), "N"),
      security);
    m_marketDataServiceInstance->GetDataStore().Store(
      MakeSequencedValue(timeAndSale, Beam::Queries::Sequence(i + 1)));
    expectedSeries.emplace_back(timestamp, timestamp + interval, price, price,
      price, price);
  }
  TimePriceQueryResult result = m_clientProtocol->SendRequest<
    LoadSecurityTimePriceSeriesService>(security, startTime, endTime, interval);
  CPPUNIT_ASSERT(result.series == expectedSeries);
}
