#ifndef NEXUS_BACKTESTER_ENVIRONMENT_HPP
#define NEXUS_BACKTESTER_ENVIRONMENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/Backtester/BacktesterHistoricalDataStore.hpp"
#include "Nexus/Backtester/BacktesterMarketDataService.hpp"
#include "Nexus/DefinitionsServiceTests/DefinitionsServiceTestEnvironment.hpp"
#include "Nexus/MarketDataService/ClientHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "Nexus/SimulationMatcher/SimulationOrderExecutionDriver.hpp"

namespace Nexus {

  /** Maintains the state needed to run the historical backtester. */
  class BacktesterEnvironment : private boost::noncopyable {
    public:

      /**
       * Constructs a BacktesterEnvironment.
       * @param startTime The backtester's starting time.
       * @param serviceClients The ServiceClients connected to the historical
       *        data source.
       */
      BacktesterEnvironment(boost::posix_time::ptime startTime,
        Beam::Ref<VirtualServiceClients> serviceClients);

      /**
       * Constructs a BacktesterEnvironment.
       * @param startTime The backtester's starting time.
       * @param endTime The backtester's ending time.
       * @param serviceClients The ServiceClients connected to the historical
       *        data source.
       */
      BacktesterEnvironment(boost::posix_time::ptime startTime,
        boost::posix_time::ptime endTime,
        Beam::Ref<VirtualServiceClients> serviceClients);

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

      /** Returns the DefinitionsServiceTestEnvironment. */
      DefinitionsService::Tests::DefinitionsServiceTestEnvironment&
        GetDefinitionsEnvironment();

      /** Returns the AdministrationServiceTestEnvironment. */
      AdministrationService::Tests::AdministrationServiceTestEnvironment&
        GetAdministrationEnvironment();

      /** Returns the MarketDataServiceTestEnvironment. */
      MarketDataService::Tests::MarketDataServiceTestEnvironment&
        GetMarketDataEnvironment();

      /** Returns the OrderExecutionServiceTestEnvironment. */
      OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment&
        GetOrderExecutionEnvironment();

      /** Returns the BacktesterMarketDataService. */
      BacktesterMarketDataService& GetMarketDataService();

      /** Returns the BacktesterMarketDataService. */
      const BacktesterMarketDataService& GetMarketDataService() const;

      void Open();

      void Close();

    private:
      VirtualServiceClients* m_serviceClients;
      BacktesterEventHandler m_eventHandler;
      boost::optional<BacktesterMarketDataService> m_marketDataService;
      Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment
        m_serviceLocatorEnvironment;
      std::unique_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        m_serviceLocatorClient;
      Beam::UidService::Tests::UidServiceTestEnvironment m_uidEnvironment;
      boost::optional<
        DefinitionsService::Tests::DefinitionsServiceTestEnvironment>
        m_definitionsEnvironment;
      boost::optional<
        AdministrationService::Tests::AdministrationServiceTestEnvironment>
        m_administrationEnvironment;
      boost::optional<
        MarketDataService::Tests::MarketDataServiceTestEnvironment>
        m_marketDataEnvironment;
      boost::optional<
        OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment>
        m_orderExecutionEnvironment;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline BacktesterEnvironment::BacktesterEnvironment(
    boost::posix_time::ptime startTime,
    Beam::Ref<VirtualServiceClients> serviceClients)
    : BacktesterEnvironment(startTime, boost::posix_time::pos_infin,
        Beam::Ref(serviceClients)) {}

  inline BacktesterEnvironment::BacktesterEnvironment(
    boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
    Beam::Ref<VirtualServiceClients> serviceClients)
    : m_serviceClients(serviceClients.Get()),
      m_eventHandler(startTime, endTime) {}

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

  inline DefinitionsService::Tests::DefinitionsServiceTestEnvironment&
      BacktesterEnvironment::GetDefinitionsEnvironment() {
    return *m_definitionsEnvironment;
  }

  inline AdministrationService::Tests::AdministrationServiceTestEnvironment&
      BacktesterEnvironment::GetAdministrationEnvironment() {
    return *m_administrationEnvironment;
  }

  inline MarketDataService::Tests::MarketDataServiceTestEnvironment&
      BacktesterEnvironment::GetMarketDataEnvironment() {
    return *m_marketDataEnvironment;
  }

  inline OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment&
      BacktesterEnvironment::GetOrderExecutionEnvironment() {
    return *m_orderExecutionEnvironment;
  }

  inline BacktesterMarketDataService&
      BacktesterEnvironment::GetMarketDataService() {
    return *m_marketDataService;
  }

  inline const BacktesterMarketDataService&
      BacktesterEnvironment::GetMarketDataService() const {
    return *m_marketDataService;
  }

  inline void BacktesterEnvironment::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_serviceClients->Open();
      m_eventHandler.Open();
      m_serviceLocatorEnvironment.Open();
      m_serviceLocatorClient = m_serviceLocatorEnvironment.BuildClient();
      m_serviceLocatorClient->SetCredentials("root", "");
      m_serviceLocatorClient->Open();
      auto rootAccount = m_serviceLocatorClient->GetAccount();
      m_uidEnvironment.Open();
      auto definitionsServiceLocatorClient =
        m_serviceLocatorEnvironment.BuildClient();
      definitionsServiceLocatorClient->SetCredentials("root", "");
      definitionsServiceLocatorClient->Open();
      m_definitionsEnvironment.emplace(
        std::move(definitionsServiceLocatorClient));
      m_definitionsEnvironment->Open();
      auto administrationServiceLocatorClient =
        m_serviceLocatorEnvironment.BuildClient();
      administrationServiceLocatorClient->SetCredentials("root", "");
      administrationServiceLocatorClient->Open();
      m_administrationEnvironment.emplace(
        std::move(administrationServiceLocatorClient));
      m_administrationEnvironment->Open();
      auto marketDataServiceLocatorClient =
        m_serviceLocatorEnvironment.BuildClient();
      marketDataServiceLocatorClient->SetCredentials("root", "");
      marketDataServiceLocatorClient->Open();
      auto marketDataAdministrationClient =
        m_administrationEnvironment->BuildClient(
        Beam::Ref(*marketDataServiceLocatorClient));
      auto historicalDataStore =
        MarketDataService::MakeVirtualHistoricalDataStore(
        std::make_unique<BacktesterHistoricalDataStore<
        MarketDataService::ClientHistoricalDataStore<
        MarketDataService::VirtualMarketDataClient*>>>(
        &m_serviceClients->GetMarketDataClient(),
        m_eventHandler.GetStartTime()));
      m_marketDataEnvironment.emplace(
        std::move(marketDataServiceLocatorClient),
        std::move(marketDataAdministrationClient),
        std::move(historicalDataStore));
      m_marketDataEnvironment->Open();
      m_marketDataService.emplace(Beam::Ref(m_eventHandler),
        Beam::Ref(*m_marketDataEnvironment),
        Beam::Ref(m_serviceClients->GetMarketDataClient()));
      auto orderExecutionServiceLocatorClient =
        m_serviceLocatorEnvironment.BuildClient();
      orderExecutionServiceLocatorClient->SetCredentials("root", "");
      orderExecutionServiceLocatorClient->Open();
      auto uidClient = m_uidEnvironment.BuildClient();
      uidClient->Open();
      auto administrationClient = m_administrationEnvironment->BuildClient(
        Beam::Ref(*m_serviceLocatorClient));
      administrationClient->Open();
      m_serviceLocatorClient->Associate(rootAccount,
        administrationClient->LoadAdministratorsRootEntry());
      m_serviceLocatorClient->Associate(rootAccount,
        administrationClient->LoadServicesRootEntry());
      auto driverMarketDataClient =
        MarketDataService::MakeVirtualMarketDataClient(
        std::make_unique<BacktesterMarketDataClient>(
        Beam::Ref(*m_marketDataService),
        m_marketDataEnvironment->BuildClient(Beam::Ref(
        *orderExecutionServiceLocatorClient))));
      auto driverTimeClient = Beam::TimeService::MakeVirtualTimeClient<
        BacktesterTimeClient>(Beam::Initialize(Beam::Ref(m_eventHandler)));
      auto driver = OrderExecutionService::MakeVirtualOrderExecutionDriver(
        std::make_unique<OrderExecutionService::SimulationOrderExecutionDriver<
        std::unique_ptr<MarketDataService::VirtualMarketDataClient>,
        std::unique_ptr<Beam::TimeService::VirtualTimeClient>>>(
        std::move(driverMarketDataClient), std::move(driverTimeClient)));
      auto orderTimeClient = Beam::TimeService::MakeVirtualTimeClient<
        BacktesterTimeClient>(Beam::Initialize(Beam::Ref(m_eventHandler)));
      m_orderExecutionEnvironment.emplace(GetDefaultMarketDatabase(),
        GetDefaultDestinationDatabase(),
        std::move(orderExecutionServiceLocatorClient), std::move(uidClient),
        std::move(administrationClient), std::move(orderTimeClient),
        std::move(driver));
      m_orderExecutionEnvironment->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void BacktesterEnvironment::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void BacktesterEnvironment::Shutdown() {
    m_eventHandler.Close();
    m_orderExecutionEnvironment.reset();
    m_marketDataEnvironment.reset();
    m_administrationEnvironment.reset();
    m_definitionsEnvironment.reset();
    m_uidEnvironment.Close();
    m_serviceLocatorClient.reset();
    m_serviceLocatorEnvironment.Close();
    m_openState.SetClosed();
  }
}

#endif
