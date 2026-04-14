#ifndef NEXUS_BACKTESTER_ENVIRONMENT_HPP
#define NEXUS_BACKTESTER_ENVIRONMENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
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
      Beam::Tests::ServiceLocatorTestEnvironment&
        get_service_locator_environment();

      /** Returns the UidServiceTestEnvironment. */
      Beam::Tests::UidServiceTestEnvironment& get_uid_environment();

      /** Returns the DefinitionsServiceTestEnvironment. */
      Tests::DefinitionsServiceTestEnvironment& get_definitions_environment();

      /** Returns the AdministrationServiceTestEnvironment. */
      Tests::AdministrationServiceTestEnvironment&
        get_administration_environment();

      /** Returns the MarketDataServiceTestEnvironment. */
      Tests::MarketDataServiceTestEnvironment& get_market_data_environment();

      /** Returns the BacktesterMarketDataService. */
      BacktesterMarketDataService& get_market_data_service();

      /** Returns the BacktesterMarketDataService. */
      const BacktesterMarketDataService& get_market_data_service() const;

      /** Returns the ChartingServiceTestEnvironment. */
      Tests::ChartingServiceTestEnvironment& get_charting_environment();

      /** Returns the ComplianceTestEnvironment. */
      Tests::ComplianceTestEnvironment& get_compliance_environment();

      /** Returns the OrderExecutionServiceTestEnvironment. */
      Tests::OrderExecutionServiceTestEnvironment&
        get_order_execution_environment();

      /** Returns the RiskServiceTestEnvironment. */
      Tests::RiskServiceTestEnvironment& get_risk_environment();

      void close();

    private:
      Clients m_clients;
      BacktesterEventHandler m_event_handler;
      Beam::TimeClient m_time_client;
      Beam::Tests::ServiceLocatorTestEnvironment m_service_locator_environment;
      Beam::ServiceLocatorClient m_service_locator_client;
      Beam::Tests::UidServiceTestEnvironment m_uid_environment;
      Beam::UidClient m_uid_client;
      Tests::DefinitionsServiceTestEnvironment m_definitions_environment;
      Tests::AdministrationServiceTestEnvironment m_administration_environment;
      AdministrationClient m_administration_client;
      Tests::MarketDataServiceTestEnvironment m_market_data_environment;
      BacktesterMarketDataService m_market_data_service;
      MarketDataClient m_market_data_client;
      Tests::ChartingServiceTestEnvironment m_charting_environment;
      Tests::ComplianceTestEnvironment m_compliance_environment;
      boost::optional<Tests::OrderExecutionServiceTestEnvironment>
        m_order_execution_environment;
      boost::optional<OrderExecutionClient> m_order_execution_client;
      boost::optional<Tests::RiskServiceTestEnvironment> m_risk_environment;
      Beam::OpenState m_open_state;

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
        m_service_locator_client(m_service_locator_environment.make_client()),
        m_uid_client(m_uid_environment.make_client()),
        m_definitions_environment(m_service_locator_client),
        m_administration_environment(m_service_locator_client),
        m_administration_client(m_administration_environment.make_client(
          Beam::Ref(m_service_locator_client))),
        m_market_data_environment(
          m_service_locator_client, m_administration_client,
          HistoricalDataStore(std::in_place_type<CutoffHistoricalDataStore<
            ClientHistoricalDataStore<MarketDataClient>>>,
            m_clients.get_market_data_client(),
            m_event_handler.get_start_time())),
        m_market_data_service(
          Beam::Ref(m_event_handler), Beam::Ref(m_market_data_environment),
          m_clients.get_market_data_client()),
        m_market_data_client(std::in_place_type<BacktesterMarketDataClient>,
          Beam::Ref(m_market_data_service),
          m_market_data_environment.make_registry_client(
            Beam::Ref(m_service_locator_client))),
        m_charting_environment(m_service_locator_client, m_market_data_client),
        m_compliance_environment(
          m_service_locator_client, m_administration_client, m_time_client) {
    try {
      auto definitions_client = m_definitions_environment.make_client(
        Beam::Ref(m_service_locator_client));
      m_order_execution_environment.emplace(
        definitions_client.load_venue_database(),
        definitions_client.load_destination_database(),
        m_service_locator_client, m_uid_client, m_administration_client,
        m_time_client, OrderExecutionDriver(
          std::in_place_type<SimulationOrderExecutionDriver<
            MarketDataClient, Beam::TimeClient>>,
          m_market_data_client, m_time_client));
      m_order_execution_client.emplace(
        m_order_execution_environment->make_client(
          Beam::Ref(m_service_locator_client)));
      auto transition_timer_factory =
        std::bind_front(boost::factory<std::unique_ptr<Beam::Timer>>(),
          std::in_place_type<Beam::TriggerTimer>);
      m_risk_environment.emplace(m_service_locator_client,
        m_administration_client, m_market_data_client,
        *m_order_execution_client, transition_timer_factory, m_time_client,
        ExchangeRateTable(definitions_client.load_exchange_rates()),
        definitions_client.load_destination_database());
      auto root_account = m_service_locator_client.get_account();
      m_service_locator_client.associate(root_account,
        m_administration_client.load_administrators_root_entry());
      m_service_locator_client.associate(root_account,
        m_administration_client.load_services_root_entry());
    } catch(const std::exception&) {
      close();
      throw;
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

  inline Beam::Tests::ServiceLocatorTestEnvironment&
      BacktesterEnvironment::get_service_locator_environment() {
    return m_service_locator_environment;
  }

  inline Beam::Tests::UidServiceTestEnvironment&
      BacktesterEnvironment::get_uid_environment() {
    return m_uid_environment;
  }

  inline Tests::DefinitionsServiceTestEnvironment&
      BacktesterEnvironment::get_definitions_environment() {
    return m_definitions_environment;
  }

  inline Tests::AdministrationServiceTestEnvironment&
      BacktesterEnvironment::get_administration_environment() {
    return m_administration_environment;
  }

  inline Tests::MarketDataServiceTestEnvironment&
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

  inline Tests::ChartingServiceTestEnvironment&
      BacktesterEnvironment::get_charting_environment() {
    return m_charting_environment;
  }

  inline Tests::ComplianceTestEnvironment&
      BacktesterEnvironment::get_compliance_environment() {
    return m_compliance_environment;
  }

  inline Tests::OrderExecutionServiceTestEnvironment&
      BacktesterEnvironment::get_order_execution_environment() {
    return *m_order_execution_environment;
  }

  inline Tests::RiskServiceTestEnvironment&
      BacktesterEnvironment::get_risk_environment() {
    return *m_risk_environment;
  }

  inline void BacktesterEnvironment::close() {
    if(m_open_state.set_closing()) {
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
    m_uid_client.close();
    m_uid_environment.close();
    m_service_locator_client.close();
    m_service_locator_environment.close();
    m_time_client.close();
    m_event_handler.close();
    m_open_state.close();
  }
}

#endif
