#ifndef NEXUS_RISK_SERVICE_TEST_ENVIRONMENT_HPP
#define NEXUS_RISK_SERVICE_TEST_ENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Queues/ConverterQueueReader.hpp>
#include <Beam/Queues/FilteredQueueReader.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/Threading/TimerBox.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/RiskServlet.hpp"
#include "Nexus/RiskService/ServiceRiskClient.hpp"

namespace Nexus::RiskService::Tests {

  /**
   * Wraps most components needed to run an instance of the RiskService with
   * helper functions.
   */
  class RiskServiceTestEnvironment {
    public:

      /**
       * Constructs a RiskServiceTestEnvironment.
       * @param service_locator_client The ServiceLocatorClient to use.
       * @param administration_client The AdministrationClient to use.
       * @param market_data_client The MarketDataClient to use.
       * @param order_execution_client The OrderExecutionClient to use.
       * @param transition_timer_factory The transition Timer to use.
       * @param time_client The TimeClient to use.
       * @param exchange_rates The exchange rates to use.
       * @param venues The venues available to the portfolio.
       * @param destinations The destinations available to flatten positions.
       */
      RiskServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client,
        AdministrationService::AdministrationClient administration_client,
        MarketDataService::MarketDataClient market_data_client,
        OrderExecutionService::OrderExecutionClient order_execution_client,
        std::function<std::unique_ptr<Beam::Threading::TimerBox> ()>
          transition_timer_factory,
        Beam::TimeService::TimeClientBox time_client,
        ExchangeRateTable exchange_rates, VenueDatabase venues,
        DestinationDatabase destinations);

      ~RiskServiceTestEnvironment();

      /**
       * Returns a new RiskClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate the RiskClient.
       */
      RiskClient make_client(
        Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client);

      void close();

    private:
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
          Beam::ServiceLocator::MetaAuthenticationServletAdapter<
            MetaRiskServlet<AdministrationService::AdministrationClient,
              MarketDataService::MarketDataClient,
              OrderExecutionService::OrderExecutionClient,
              Beam::Threading::TimerBox,
              Beam::TimeService::TimeClientBox, LocalRiskDataStore*>,
            Beam::ServiceLocator::ServiceLocatorClientBox>,
          ServerConnection*,
          Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
          Beam::Codecs::NullEncoder,
          std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
          Beam::ServiceLocator::ServiceLocatorClientBox,
          Beam::Services::MessageProtocol<std::unique_ptr<ClientChannel>,
            Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
            Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;
      LocalRiskDataStore m_data_store;
      ServerConnection m_server_connection;
      boost::optional<ServiceProtocolServletContainer> m_container;

      RiskServiceTestEnvironment(const RiskServiceTestEnvironment&) = delete;
      RiskServiceTestEnvironment& operator =(
        const RiskServiceTestEnvironment&) = delete;
  };

  inline RiskServiceTestEnvironment::RiskServiceTestEnvironment(
      Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client,
      AdministrationService::AdministrationClient administration_client,
      MarketDataService::MarketDataClient market_data_client,
      OrderExecutionService::OrderExecutionClient order_execution_client,
      std::function<std::unique_ptr<Beam::Threading::TimerBox> ()>
        transition_timer_factory, Beam::TimeService::TimeClientBox time_client,
      ExchangeRateTable exchange_rates, VenueDatabase venues,
      DestinationDatabase destinations) {
    auto accounts =
      std::make_shared<Beam::Queue<Beam::ServiceLocator::AccountUpdate>>();
    service_locator_client.MonitorAccounts(accounts);
    m_container.emplace(Beam::Initialize(service_locator_client,
      Beam::Initialize(Beam::MakeConverterQueueReader(
        Beam::MakeFilteredQueueReader(std::move(accounts),
          [] (const auto& update) {
            return update.m_type ==
              Beam::ServiceLocator::AccountUpdate::Type::ADDED;
          }),
          [] (const auto& update) {
            return update.m_account;
          }),
        std::move(administration_client), std::move(market_data_client),
        std::move(order_execution_client), std::move(transition_timer_factory),
        std::move(time_client), &m_data_store, std::move(exchange_rates),
        std::move(venues), std::move(destinations))), &m_server_connection,
      boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>());
  }

  inline RiskServiceTestEnvironment::~RiskServiceTestEnvironment() {
    close();
  }

  inline RiskClient RiskServiceTestEnvironment::make_client(
      Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client) {
    return RiskClient(
      std::in_place_type<ServiceRiskClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(std::move(service_locator_client),
        std::bind_front(boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
          "test_risk_client", std::ref(m_server_connection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void RiskServiceTestEnvironment::close() {
    m_container->Close();
  }
}

#endif
