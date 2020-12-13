#ifndef NEXUS_BACKTESTER_ENVIRONMENT_HPP
#define NEXUS_BACKTESTER_ENVIRONMENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/RegistryServiceTests/RegistryServiceTestEnvironment.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/Backtester/BacktesterHistoricalDataStore.hpp"
#include "Nexus/Backtester/BacktesterMarketDataService.hpp"
#include "Nexus/ChartingServiceTests/ChartingServiceTestEnvironment.hpp"
#include "Nexus/ComplianceTests/ComplianceTestEnvironment.hpp"
#include "Nexus/DefinitionsServiceTests/DefinitionsServiceTestEnvironment.hpp"
#include "Nexus/MarketDataService/ClientHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/RiskServiceTests/RiskServiceTestEnvironment.hpp"
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "Nexus/SimulationMatcher/SimulationOrderExecutionDriver.hpp"

namespace Nexus {

  /** Maintains the state needed to run the historical backtester. */
  class BacktesterEnvironment {
    public:

      /**
       * Constructs a BacktesterEnvironment.
       * @param startTime The backtester's starting time.
       * @param serviceClients The ServiceClients connected to the historical
       *        data source.
       */
      BacktesterEnvironment(boost::posix_time::ptime startTime,
        ServiceClientsBox serviceClients);

      /**
       * Constructs a BacktesterEnvironment.
       * @param startTime The backtester's starting time.
       * @param endTime The backtester's ending time.
       * @param serviceClients The ServiceClients connected to the historical
       *        data source.
       */
      BacktesterEnvironment(boost::posix_time::ptime startTime,
        boost::posix_time::ptime endTime, ServiceClientsBox serviceClients);

      ~BacktesterEnvironment();

      /** Returns the BacktesterEventHandler. */
      const BacktesterEventHandler& GetEventHandler() const;

      /** Returns the BacktesterEventHandler. */
      BacktesterEventHandler& GetEventHandler();

      /** Returns the ServiceLocatorTestEnvironment. */
      Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment&
        GetServiceLocatorEnvironment();

      /** Returns the UidServiceTestEnvironment. */
      Beam::UidService::Tests::UidServiceTestEnvironment& GetUidEnvironment();

      /** Returns the RegistryServiceTestEnvironment. */
      Beam::RegistryService::Tests::RegistryServiceTestEnvironment&
        GetRegistryEnvironment();

      /** Returns the DefinitionsServiceTestEnvironment. */
      DefinitionsService::Tests::DefinitionsServiceTestEnvironment&
        GetDefinitionsEnvironment();

      /** Returns the AdministrationServiceTestEnvironment. */
      AdministrationService::Tests::AdministrationServiceTestEnvironment&
        GetAdministrationEnvironment();

      /** Returns the MarketDataServiceTestEnvironment. */
      MarketDataService::Tests::MarketDataServiceTestEnvironment&
        GetMarketDataEnvironment();

      /** Returns the BacktesterMarketDataService. */
      BacktesterMarketDataService& GetMarketDataService();

      /** Returns the BacktesterMarketDataService. */
      const BacktesterMarketDataService& GetMarketDataService() const;

      /** Returns the ChartingServiceTestEnvironment. */
      ChartingService::Tests::ChartingServiceTestEnvironment&
        GetChartingEnvironment();

      /** Returns the ComplianceTestEnvironment. */
      Compliance::Tests::ComplianceTestEnvironment& GetComplianceEnvironment();

      /** Returns the OrderExecutionServiceTestEnvironment. */
      OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment&
        GetOrderExecutionEnvironment();

      /** Returns the RiskServiceTestEnvironment. */
      RiskService::Tests::RiskServiceTestEnvironment& GetRiskEnvironment();

      void Close();

    private:
      ServiceClientsBox m_serviceClients;
      BacktesterEventHandler m_eventHandler;
      Beam::TimeService::TimeClientBox m_timeClient;
      Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment
        m_serviceLocatorEnvironment;
      Beam::ServiceLocator::ServiceLocatorClientBox m_serviceLocatorClient;
      Beam::UidService::Tests::UidServiceTestEnvironment m_uidEnvironment;
      Beam::UidService::UidClientBox m_uidClient;
      Beam::RegistryService::Tests::RegistryServiceTestEnvironment
        m_registryEnvironment;
      DefinitionsService::Tests::DefinitionsServiceTestEnvironment
        m_definitionsEnvironment;
      AdministrationService::Tests::AdministrationServiceTestEnvironment
        m_administrationEnvironment;
      AdministrationService::AdministrationClientBox m_administrationClient;
      MarketDataService::Tests::MarketDataServiceTestEnvironment
        m_marketDataEnvironment;
      BacktesterMarketDataService m_marketDataService;
      MarketDataService::MarketDataClientBox m_marketDataClient;
      ChartingService::Tests::ChartingServiceTestEnvironment
        m_chartingEnvironment;
      Compliance::Tests::ComplianceTestEnvironment m_complianceEnvironment;
      boost::optional<
        OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment>
        m_orderExecutionEnvironment;
      boost::optional<OrderExecutionService::OrderExecutionClientBox>
        m_orderExecutionClient;
      boost::optional<RiskService::Tests::RiskServiceTestEnvironment>
        m_riskEnvironment;
      Beam::IO::OpenState m_openState;

      BacktesterEnvironment(const BacktesterEnvironment&) = delete;
      BacktesterEnvironment& operator =(const BacktesterEnvironment&) = delete;
  };

  inline BacktesterEnvironment::BacktesterEnvironment(
    boost::posix_time::ptime startTime, ServiceClientsBox serviceClients)
    : BacktesterEnvironment(startTime, boost::posix_time::pos_infin,
        std::move(serviceClients)) {}

  inline BacktesterEnvironment::BacktesterEnvironment(
      boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
      ServiceClientsBox serviceClients)
      : m_serviceClients(std::move(serviceClients)),
        m_eventHandler(startTime, endTime),
        m_timeClient(std::in_place_type<BacktesterTimeClient>,
          Beam::Ref(m_eventHandler)),
        m_serviceLocatorClient(m_serviceLocatorEnvironment.MakeClient()),
        m_uidClient(m_uidEnvironment.MakeClient()),
        m_registryEnvironment(m_serviceLocatorClient),
        m_definitionsEnvironment(m_serviceLocatorClient),
        m_administrationEnvironment(m_serviceLocatorClient),
        m_administrationClient(m_administrationEnvironment.MakeClient(
          m_serviceLocatorClient)),
        m_marketDataEnvironment(m_serviceLocatorClient, m_administrationClient,
          MarketDataService::HistoricalDataStoreBox(
            std::in_place_type<BacktesterHistoricalDataStore<
              MarketDataService::ClientHistoricalDataStore<
                MarketDataService::MarketDataClientBox>>>,
            m_serviceClients.GetMarketDataClient(),
            m_eventHandler.GetStartTime())),
        m_marketDataService(Beam::Ref(m_eventHandler),
          Beam::Ref(m_marketDataEnvironment),
          m_serviceClients.GetMarketDataClient()),
        m_marketDataClient(std::make_unique<BacktesterMarketDataClient>(
          Beam::Ref(m_marketDataService), m_marketDataEnvironment.MakeClient(
            m_serviceLocatorClient))),
        m_chartingEnvironment(m_serviceLocatorClient, m_marketDataClient),
        m_complianceEnvironment(m_serviceLocatorClient, m_administrationClient,
          m_timeClient) {
    try {
      auto definitionsClient = m_definitionsEnvironment.MakeClient(
        m_serviceLocatorClient);
      m_orderExecutionEnvironment.emplace(
        definitionsClient.LoadMarketDatabase(),
        definitionsClient.LoadDestinationDatabase(), m_serviceLocatorClient,
        m_uidClient, m_administrationClient, m_timeClient,
        OrderExecutionService::MakeVirtualOrderExecutionDriver(
          std::make_unique<
            OrderExecutionService::SimulationOrderExecutionDriver<
              MarketDataService::MarketDataClientBox,
              Beam::TimeService::TimeClientBox>>(
                m_marketDataClient, m_timeClient)));
      m_orderExecutionClient.emplace(m_orderExecutionEnvironment->MakeClient(
        m_serviceLocatorClient));
      auto transitionTimerFactory = std::bind(
        boost::factory<std::unique_ptr<Beam::Threading::TimerBox>>(),
        std::in_place_type<Beam::Threading::TriggerTimer>);
      m_riskEnvironment.emplace(m_serviceLocatorClient, m_administrationClient,
        m_marketDataClient, *m_orderExecutionClient, transitionTimerFactory,
        m_timeClient, definitionsClient.LoadExchangeRates(),
        definitionsClient.LoadMarketDatabase(),
        definitionsClient.LoadDestinationDatabase());
      auto rootAccount = m_serviceLocatorClient.GetAccount();
      m_serviceLocatorClient.Associate(rootAccount,
        m_administrationClient.LoadAdministratorsRootEntry());
      m_serviceLocatorClient.Associate(rootAccount,
        m_administrationClient.LoadServicesRootEntry());
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  inline BacktesterEnvironment::~BacktesterEnvironment() {
    Close();
  }

  inline const BacktesterEventHandler&
      BacktesterEnvironment::GetEventHandler() const {
    return m_eventHandler;
  }

  inline BacktesterEventHandler& BacktesterEnvironment::GetEventHandler() {
    return m_eventHandler;
  }

  inline Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment&
      BacktesterEnvironment::GetServiceLocatorEnvironment() {
    return m_serviceLocatorEnvironment;
  }

  inline Beam::UidService::Tests::UidServiceTestEnvironment&
      BacktesterEnvironment::GetUidEnvironment() {
    return m_uidEnvironment;
  }

  inline Beam::RegistryService::Tests::RegistryServiceTestEnvironment&
      BacktesterEnvironment::GetRegistryEnvironment() {
    return m_registryEnvironment;
  }

  inline DefinitionsService::Tests::DefinitionsServiceTestEnvironment&
      BacktesterEnvironment::GetDefinitionsEnvironment() {
    return m_definitionsEnvironment;
  }

  inline AdministrationService::Tests::AdministrationServiceTestEnvironment&
      BacktesterEnvironment::GetAdministrationEnvironment() {
    return m_administrationEnvironment;
  }

  inline MarketDataService::Tests::MarketDataServiceTestEnvironment&
      BacktesterEnvironment::GetMarketDataEnvironment() {
    return m_marketDataEnvironment;
  }

  inline BacktesterMarketDataService&
      BacktesterEnvironment::GetMarketDataService() {
    return m_marketDataService;
  }

  inline const BacktesterMarketDataService&
      BacktesterEnvironment::GetMarketDataService() const {
    return m_marketDataService;
  }

  inline ChartingService::Tests::ChartingServiceTestEnvironment&
      BacktesterEnvironment::GetChartingEnvironment() {
    return m_chartingEnvironment;
  }

  inline Compliance::Tests::ComplianceTestEnvironment&
      BacktesterEnvironment::GetComplianceEnvironment() {
    return m_complianceEnvironment;
  }

  inline OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment&
      BacktesterEnvironment::GetOrderExecutionEnvironment() {
    return *m_orderExecutionEnvironment;
  }

  inline RiskService::Tests::RiskServiceTestEnvironment&
      BacktesterEnvironment::GetRiskEnvironment() {
    return *m_riskEnvironment;
  }

  inline void BacktesterEnvironment::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_riskEnvironment->Close();
    m_orderExecutionClient->Close();
    m_orderExecutionEnvironment->Close();
    m_complianceEnvironment.Close();
    m_chartingEnvironment.Close();
    m_marketDataClient.Close();
    m_marketDataEnvironment.Close();
    m_administrationClient.Close();
    m_administrationEnvironment.Close();
    m_definitionsEnvironment.Close();
    m_registryEnvironment.Close();
    m_uidClient.Close();
    m_uidEnvironment.Close();
    m_serviceLocatorClient.Close();
    m_serviceLocatorEnvironment.Close();
    m_timeClient.Close();
    m_eventHandler.Close();
    m_openState.Close();
  }
}

#endif
