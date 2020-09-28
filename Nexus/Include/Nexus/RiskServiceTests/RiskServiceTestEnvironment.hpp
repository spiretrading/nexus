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
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/Threading/VirtualTimer.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
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
        std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient,
        std::shared_ptr<AdministrationService::VirtualAdministrationClient>
        administrationClient,
        std::shared_ptr<MarketDataService::VirtualMarketDataClient>
        marketDataClient,
        std::shared_ptr<OrderExecutionService::VirtualOrderExecutionClient>
        orderExecutionClient,
        std::function<std::unique_ptr<Beam::Threading::VirtualTimer> ()>
        transitionTimerFactory,
        std::shared_ptr<Beam::TimeService::VirtualTimeClient> timeClient,
        std::vector<ExchangeRate> exchangeRates, MarketDatabase markets,
        DestinationDatabase destinations);

      ~RiskServiceTestEnvironment();

      /**
       * Builds a new RiskClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the RiskClient.
       */
      std::unique_ptr<VirtualRiskClient> BuildClient(
        Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

      /** Closes the servlet. */
      void Close();

    private:
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;
      using UidClient = Beam::UidService::VirtualUidClient;
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<MetaRiskServlet<
        std::shared_ptr<AdministrationService::VirtualAdministrationClient>,
        std::shared_ptr<MarketDataService::VirtualMarketDataClient>,
        std::shared_ptr<OrderExecutionService::VirtualOrderExecutionClient>,
        Beam::Threading::VirtualTimer,
        std::shared_ptr<Beam::TimeService::VirtualTimeClient>,
        LocalRiskDataStore*>, std::shared_ptr<ServiceLocatorClient>>,
        ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocatorClient, Beam::Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
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
      std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
      serviceLocatorClient, std::shared_ptr<
      AdministrationService::VirtualAdministrationClient> administrationClient,
      std::shared_ptr<MarketDataService::VirtualMarketDataClient>
      marketDataClient, std::shared_ptr<
      OrderExecutionService::VirtualOrderExecutionClient> orderExecutionClient,
      std::function<std::unique_ptr<Beam::Threading::VirtualTimer> ()>
      transitionTimerFactory, std::shared_ptr<
      Beam::TimeService::VirtualTimeClient> timeClient,
      std::vector<ExchangeRate> exchangeRates, MarketDatabase markets,
      DestinationDatabase destinations) {
    auto accounts = std::make_shared<
      Beam::Queue<Beam::ServiceLocator::AccountUpdate>>();
    serviceLocatorClient->MonitorAccounts(accounts);
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
      RiskServiceTestEnvironment::BuildClient(
      Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
      serviceLocatorClient) {
    auto builder = ServiceProtocolClientBuilder(Beam::Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_risk_client", m_serverConnection);
      },
      [] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    return MakeVirtualRiskClient(
      std::make_unique<RiskClient<ServiceProtocolClientBuilder>>(builder));
  }

  inline void RiskServiceTestEnvironment::Close() {
    m_container->Close();
  }
}

#endif
