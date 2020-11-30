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
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/Threading/TimerBox.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <boost/functional/factory.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/RiskClient.hpp"
#include "Nexus/RiskService/RiskServlet.hpp"
#include "Nexus/RiskService/VirtualRiskClient.hpp"
#include "Nexus/RiskServiceTests/RiskServiceTests.hpp"

namespace Nexus::RiskService::Tests {

  /**
   * Wraps most components needed to run an instance of the RiskService with
   * helper functions.
   */
  class RiskServiceTestEnvironment {
    public:

      /**
       * Constructs a RiskServiceTestEnvironment.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       * @param administrationClient The AdministrationClient to use.
       * @param marketDataClient The MarketDataClient to use.
       * @param orderExecutionClient The OrderExecutionClient to use.
       * @param transitionTimerFactory The transition Timer to use.
       * @param timeClient The TimeClient to use.
       * @param exchangeRates The list of exchange rates.
       * @param markets The market database used by the portfolio.
       * @param destinations The destination database used to flatten positions.
       */
      RiskServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
        std::shared_ptr<AdministrationService::VirtualAdministrationClient>
        administrationClient,
        std::shared_ptr<MarketDataService::VirtualMarketDataClient>
        marketDataClient,
        std::shared_ptr<OrderExecutionService::VirtualOrderExecutionClient>
        orderExecutionClient,
        std::function<std::unique_ptr<Beam::Threading::TimerBox> ()>
        transitionTimerFactory, Beam::TimeService::TimeClientBox timeClient,
        std::vector<ExchangeRate> exchangeRates, MarketDatabase markets,
        DestinationDatabase destinations);

      ~RiskServiceTestEnvironment();

      /**
       * Builds a new RiskClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the RiskClient.
       */
      std::unique_ptr<VirtualRiskClient> MakeClient(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient);

      void Close();

    private:
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
          Beam::ServiceLocator::MetaAuthenticationServletAdapter<
            MetaRiskServlet<
              std::shared_ptr<
                AdministrationService::VirtualAdministrationClient>,
              std::shared_ptr<MarketDataService::VirtualMarketDataClient>,
              std::shared_ptr<
                OrderExecutionService::VirtualOrderExecutionClient>,
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
      LocalRiskDataStore m_dataStore;
      ServerConnection m_serverConnection;
      boost::optional<ServiceProtocolServletContainer> m_container;

      RiskServiceTestEnvironment(const RiskServiceTestEnvironment&) = delete;
      RiskServiceTestEnvironment& operator =(
        const RiskServiceTestEnvironment&) = delete;
  };

  inline RiskServiceTestEnvironment::RiskServiceTestEnvironment(
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
      std::shared_ptr<AdministrationService::VirtualAdministrationClient>
      administrationClient, std::shared_ptr<
        MarketDataService::VirtualMarketDataClient> marketDataClient,
      std::shared_ptr<OrderExecutionService::VirtualOrderExecutionClient>
      orderExecutionClient,
      std::function<std::unique_ptr<Beam::Threading::TimerBox> ()>
      transitionTimerFactory, Beam::TimeService::TimeClientBox timeClient,
      std::vector<ExchangeRate> exchangeRates, MarketDatabase markets,
      DestinationDatabase destinations) {
    auto accounts = std::make_shared<
      Beam::Queue<Beam::ServiceLocator::AccountUpdate>>();
    serviceLocatorClient.MonitorAccounts(accounts);
    m_container.emplace(Beam::Initialize(serviceLocatorClient,
      Beam::Initialize(Beam::MakeConverterQueueReader(
        Beam::MakeFilteredQueueReader(std::move(accounts),
          [] (const auto& update) {
            return update.m_type ==
              Beam::ServiceLocator::AccountUpdate::Type::ADDED;
          }),
          [] (const auto& update) {
            return update.m_account;
          }), std::move(administrationClient), std::move(marketDataClient),
        std::move(orderExecutionClient), std::move(transitionTimerFactory),
        std::move(timeClient), &m_dataStore, std::move(exchangeRates),
        std::move(markets), std::move(destinations))), &m_serverConnection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>());
  }

  inline RiskServiceTestEnvironment::~RiskServiceTestEnvironment() {
    Close();
  }

  inline std::unique_ptr<VirtualRiskClient>
      RiskServiceTestEnvironment::MakeClient(
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient) {
    return MakeVirtualRiskClient(
      std::make_unique<RiskClient<ServiceProtocolClientBuilder>>(
        ServiceProtocolClientBuilder(std::move(serviceLocatorClient),
          std::bind(boost::factory<
            std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
            "test_risk_client", std::ref(m_serverConnection)),
          boost::factory<
            std::unique_ptr<ServiceProtocolClientBuilder::Timer>>())));
  }

  inline void RiskServiceTestEnvironment::Close() {
    m_container->Close();
  }
}

#endif
