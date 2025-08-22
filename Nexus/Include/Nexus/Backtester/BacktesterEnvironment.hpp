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
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/Backtester/BacktesterMarketDataClient.hpp"
#include "Nexus/Backtester/BacktesterMarketDataService.hpp"
#include "Nexus/Backtester/BacktesterTimeClient.hpp"
#include "Nexus/Backtester/CutoffHistoricalDataStore.hpp"
#include "Nexus/ChartingServiceTests/ChartingServiceTestEnvironment.hpp"
#include "Nexus/Clients/Clients.hpp"
#include "Nexus/ComplianceTests/ComplianceTestEnvironment.hpp"
#include "Nexus/DefinitionsServiceTests/DefinitionsServiceTestEnvironment.hpp"
#include "Nexus/MarketDataService/ClientHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/RiskServiceTests/RiskServiceTestEnvironment.hpp"
#include "Nexus/SimulationMatcher/SimulationOrderExecutionDriver.hpp"

namespace Nexus {

  /** Maintains the state needed to run the historical backtester. */
  class BacktesterEnvironment {
    public:

      /**
       * Constructs a BacktesterEnvironment.
       * @param start The backtester's starting time.
       * @param clients The Clients connected to the historical data source.
       */
      BacktesterEnvironment(boost::posix_time::ptime start, Clients clients);

      /**
       * Constructs a BacktesterEnvironment.
       * @param start The backtester's starting time.
       * @param end The backtester's ending time.
       * @param clients The Clients connected to the historical data source.
       */
      BacktesterEnvironment(boost::posix_time::ptime start,
        boost::posix_time::ptime end, Clients clients);

      ~BacktesterEnvironment();

      /** Returns the BacktesterEventHandler. */
      const BacktesterEventHandler& get_event_handler() const;

      /** Returns the BacktesterEventHandler. */
      BacktesterEventHandler& get_event_handler();

      /** Returns the ServiceLocatorTestEnvironment. */
      Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment&
        get_service_locator_environment();

      /** Returns the UidServiceTestEnvironment. */
      Beam::UidService::Tests::UidServiceTestEnvironment& get_uid_environment();

      /** Returns the RegistryServiceTestEnvironment. */
      Beam::RegistryService::Tests::RegistryServiceTestEnvironment&
        get_registry_environment();

      /** Returns the DefinitionsServiceTestEnvironment. */
      DefinitionsService::Tests::DefinitionsServiceTestEnvironment&
        get_definitions_environment();

      /** Returns the AdministrationServiceTestEnvironment. */
      Tests::AdministrationServiceTestEnvironment&
        get_administration_environment();

      /** Returns the MarketDataServiceTestEnvironment. */
      MarketDataService::Tests::MarketDataServiceTestEnvironment&
        get_market_data_environment();

      /** Returns the BacktesterMarketDataService. */
      BacktesterMarketDataService& get_market_data_service();

      /** Returns the BacktesterMarketDataService. */
      const BacktesterMarketDataService& get_market_data_service() const;

      /** Returns the ChartingServiceTestEnvironment. */
      ChartingService::Tests::ChartingServiceTestEnvironment&
        get_charting_environment();

      /** Returns the ComplianceTestEnvironment. */
      Compliance::Tests::ComplianceTestEnvironment&
        get_compliance_environment();

      /** Returns the OrderExecutionServiceTestEnvironment. */
      OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment&
        get_order_execution_environment();

      /** Returns the RiskServiceTestEnvironment. */
      RiskService::Tests::RiskServiceTestEnvironment& get_risk_environment();

      void close();

    private:
      Clients m_clients;
      BacktesterEventHandler m_event_handler;
      Beam::TimeService::TimeClientBox m_time_client;
      Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment
        m_service_locator_environment;
      Beam::ServiceLocator::ServiceLocatorClientBox m_service_locator_client;
      Beam::UidService::Tests::UidServiceTestEnvironment m_uid_environment;
      Beam::UidService::UidClientBox m_uid_client;
      Beam::RegistryService::Tests::RegistryServiceTestEnvironment
        m_registry_environment;
      DefinitionsService::Tests::DefinitionsServiceTestEnvironment
        m_definitions_environment;
      Tests::AdministrationServiceTestEnvironment m_administration_environment;
      AdministrationClient m_administration_client;
      MarketDataService::Tests::MarketDataServiceTestEnvironment
        m_market_data_environment;
      BacktesterMarketDataService m_market_data_service;
      MarketDataService::MarketDataClient m_market_data_client;
      ChartingService::Tests::ChartingServiceTestEnvironment
        m_charting_environment;
      Compliance::Tests::ComplianceTestEnvironment m_compliance_environment;
      boost::optional<
        OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment>
        m_order_execution_environment;
      boost::optional<OrderExecutionService::OrderExecutionClient>
        m_order_execution_client;
      boost::optional<RiskService::Tests::RiskServiceTestEnvironment>
        m_risk_environment;
      Beam::IO::OpenState m_open_state;

      BacktesterEnvironment(const BacktesterEnvironment&) = delete;
      BacktesterEnvironment& operator =(const BacktesterEnvironment&) = delete;
  };

  inline BacktesterEnvironment::BacktesterEnvironment(
    boost::posix_time::ptime start, Clients clients)
    : BacktesterEnvironment(
        start, boost::posix_time::pos_infin, std::move(clients)) {}

  inline BacktesterEnvironment::BacktesterEnvironment(
      boost::posix_time::ptime start, boost::posix_time::ptime end,
      Clients clients)
      : m_clients(std::move(clients)),
        m_event_handler(start, end),
        m_time_client(
          std::in_place_type<BacktesterTimeClient>, Beam::Ref(m_event_handler)),
        m_service_locator_client(m_service_locator_environment.MakeClient()),
        m_uid_client(m_uid_environment.MakeClient()),
        m_registry_environment(m_service_locator_client),
        m_definitions_environment(m_service_locator_client),
        m_administration_environment(m_service_locator_client),
        m_administration_client(
          m_administration_environment.make_client(m_service_locator_client)),
        m_market_data_environment(
          m_service_locator_client, m_administration_client,
          MarketDataService::HistoricalDataStore(
            std::in_place_type<CutoffHistoricalDataStore<
              MarketDataService::ClientHistoricalDataStore<
                MarketDataService::MarketDataClient>>>,
            m_clients.get_market_data_client(),
            m_event_handler.get_start_time())),
        m_market_data_service(Beam::Ref(m_event_handler),
          Beam::Ref(m_market_data_environment),
          m_clients.get_market_data_client()),
        m_market_data_client(std::in_place_type<BacktesterMarketDataClient>,
          Beam::Ref(m_market_data_service),
          m_market_data_environment.make_registry_client(
            m_service_locator_client)),
        m_charting_environment(m_service_locator_client, m_market_data_client),
        m_compliance_environment(
          m_service_locator_client, m_administration_client, m_time_client) {
    try {
      auto definitions_client =
        m_definitions_environment.make_client(m_service_locator_client);
      m_order_execution_environment.emplace(
        definitions_client.load_venue_database(),
        definitions_client.load_destination_database(),
        m_service_locator_client, m_uid_client, m_administration_client,
        m_time_client, OrderExecutionService::OrderExecutionDriver(
          std::in_place_type<
            OrderExecutionService::SimulationOrderExecutionDriver<
              MarketDataService::MarketDataClient,
              Beam::TimeService::TimeClientBox>>, m_market_data_client,
          m_time_client));
      m_order_execution_client.emplace(
        m_order_execution_environment->make_client(m_service_locator_client));
      auto transition_timer_factory = std::bind_front(
        boost::factory<std::unique_ptr<Beam::Threading::TimerBox>>(),
        std::in_place_type<Beam::Threading::TriggerTimer>);
      m_risk_environment.emplace(m_service_locator_client,
        m_administration_client, m_market_data_client,
        *m_order_execution_client, transition_timer_factory, m_time_client,
        ExchangeRateTable(definitions_client.load_exchange_rates()),
        definitions_client.load_venue_database(),
        definitions_client.load_destination_database());
      auto root_account = m_service_locator_client.GetAccount();
      m_service_locator_client.Associate(root_account,
        m_administration_client.load_administrators_root_entry());
      m_service_locator_client.Associate(root_account,
        m_administration_client.load_services_root_entry());
    } catch(const std::exception&) {
      close();
      BOOST_RETHROW;
    }
  }

  inline BacktesterEnvironment::~BacktesterEnvironment() {
    close();
  }

  inline const BacktesterEventHandler&
      BacktesterEnvironment::get_event_handler() const {
    return m_event_handler;
  }

  inline BacktesterEventHandler& BacktesterEnvironment::get_event_handler() {
    return m_event_handler;
  }

  inline Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment&
      BacktesterEnvironment::get_service_locator_environment() {
    return m_service_locator_environment;
  }

  inline Beam::UidService::Tests::UidServiceTestEnvironment&
      BacktesterEnvironment::get_uid_environment() {
    return m_uid_environment;
  }

  inline Beam::RegistryService::Tests::RegistryServiceTestEnvironment&
      BacktesterEnvironment::get_registry_environment() {
    return m_registry_environment;
  }

  inline DefinitionsService::Tests::DefinitionsServiceTestEnvironment&
      BacktesterEnvironment::get_definitions_environment() {
    return m_definitions_environment;
  }

  inline Tests::AdministrationServiceTestEnvironment&
      BacktesterEnvironment::get_administration_environment() {
    return m_administration_environment;
  }

  inline MarketDataService::Tests::MarketDataServiceTestEnvironment&
      BacktesterEnvironment::get_market_data_environment() {
    return m_market_data_environment;
  }

  inline BacktesterMarketDataService&
      BacktesterEnvironment::get_market_data_service() {
    return m_market_data_service;
  }

  inline const BacktesterMarketDataService&
      BacktesterEnvironment::get_market_data_service() const {
    return m_market_data_service;
  }

  inline ChartingService::Tests::ChartingServiceTestEnvironment&
      BacktesterEnvironment::get_charting_environment() {
    return m_charting_environment;
  }

  inline Compliance::Tests::ComplianceTestEnvironment&
      BacktesterEnvironment::get_compliance_environment() {
    return m_compliance_environment;
  }

  inline OrderExecutionService::Tests::OrderExecutionServiceTestEnvironment&
      BacktesterEnvironment::get_order_execution_environment() {
    return *m_order_execution_environment;
  }

  inline RiskService::Tests::RiskServiceTestEnvironment&
      BacktesterEnvironment::get_risk_environment() {
    return *m_risk_environment;
  }

  inline void BacktesterEnvironment::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_risk_environment->close();
    m_order_execution_client->close();
    m_order_execution_environment->close();
    m_compliance_environment.close();
    m_charting_environment.close();
    m_market_data_client.close();
    m_market_data_environment.close();
    m_administration_client.close();
    m_administration_environment.close();
    m_definitions_environment.close();
    m_registry_environment.Close();
    m_uid_client.Close();
    m_uid_environment.Close();
    m_service_locator_client.Close();
    m_service_locator_environment.Close();
    m_time_client.Close();
    m_event_handler.close();
    m_open_state.Close();
  }
}

#endif
