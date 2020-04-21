#ifndef NEXUS_APPLICATIONSERVICECLIENTS_HPP
#define NEXUS_APPLICATIONSERVICECLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/RegistryService/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/ChartingService/ApplicationDefinitions.hpp"
#include "Nexus/Compliance/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"
#include "Nexus/RiskService/ApplicationDefinitions.hpp"

namespace Nexus {

  /*! \class ApplicationServiceClients
      \brief Implements the ServiceClients interface using live application
             clients.
   */
  class ApplicationServiceClients : private boost::noncopyable {
    public:
      using ServiceLocatorClient =
        Beam::ServiceLocator::ApplicationServiceLocatorClient::Client;

      using RegistryClient =
        Beam::RegistryService::ApplicationRegistryClient::Client;

      using AdministrationClient =
        AdministrationService::ApplicationAdministrationClient::Client;

      using DefinitionsClient =
        DefinitionsService::ApplicationDefinitionsClient::Client;

      using MarketDataClient =
        MarketDataService::ApplicationMarketDataClient::Client;

      using ChartingClient =
        ChartingService::ApplicationChartingClient::Client;

      using ComplianceClient =
        Compliance::ApplicationComplianceClient::Client;

      using OrderExecutionClient =
        OrderExecutionService::ApplicationOrderExecutionClient::Client;

      using RiskClient = RiskService::ApplicationRiskClient::Client;

      using TimeClient = Beam::TimeService::LiveNtpTimeClient;

      using Timer = Beam::Threading::LiveTimer;

      //! Constructs an ApplicationServiceClients.
      /*!
        \param address The IpAddress to connect to.
        \param username The client's username.
        \param password The client's password.
        \param socketThreadPool The SocketThreadPool to use by the clients.
        \param timerThreadPool The TimerThreadPool to use by the clients.
      */
      ApplicationServiceClients(const Beam::Network::IpAddress& address,
        const std::string& username, const std::string& password,
        Beam::Ref<Beam::Network::SocketThreadPool> socketThreadPool,
        Beam::Ref<Beam::Threading::TimerThreadPool> timerThreadPool);

      ~ApplicationServiceClients();

      ServiceLocatorClient& GetServiceLocatorClient();

      RegistryClient& GetRegistryClient();

      AdministrationClient& GetAdministrationClient();

      DefinitionsClient& GetDefinitionsClient();

      MarketDataClient& GetMarketDataClient();

      ChartingClient& GetChartingClient();

      ComplianceClient& GetComplianceClient();

      OrderExecutionClient& GetOrderExecutionClient();

      RiskClient& GetRiskClient();

      TimeClient& GetTimeClient();

      std::unique_ptr<Timer> BuildTimer(
        boost::posix_time::time_duration duration);

      void Open();

      void Close();

    private:
      Beam::Network::IpAddress m_address;
      std::string m_username;
      std::string m_password;
      Beam::Network::SocketThreadPool* m_socketThreadPool;
      Beam::Threading::TimerThreadPool* m_timerThreadPool;
      Beam::ServiceLocator::ApplicationServiceLocatorClient
        m_serviceLocatorClient;
      Beam::Remote<Beam::RegistryService::ApplicationRegistryClient,
        Beam::Threading::Mutex> m_registryClient;
      Beam::Remote<AdministrationService::ApplicationAdministrationClient,
        Beam::Threading::Mutex> m_administrationClient;
      Beam::Remote<DefinitionsService::ApplicationDefinitionsClient,
        Beam::Threading::Mutex> m_definitionsClient;
      Beam::Remote<MarketDataService::ApplicationMarketDataClient,
        Beam::Threading::Mutex> m_marketDataClient;
      Beam::Remote<ChartingService::ApplicationChartingClient,
        Beam::Threading::Mutex> m_chartingClient;
      Beam::Remote<Compliance::ApplicationComplianceClient,
        Beam::Threading::Mutex> m_complianceClient;
      Beam::Remote<OrderExecutionService::ApplicationOrderExecutionClient,
        Beam::Threading::Mutex> m_orderExecutionClient;
      Beam::Remote<RiskService::ApplicationRiskClient, Beam::Threading::Mutex>
        m_riskClient;
      Beam::Remote<std::unique_ptr<TimeClient>, Beam::Threading::Mutex>
        m_timeClient;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline ApplicationServiceClients::ApplicationServiceClients(
      const Beam::Network::IpAddress& address, const std::string& username,
      const std::string& password,
      Beam::Ref<Beam::Network::SocketThreadPool> socketThreadPool,
      Beam::Ref<Beam::Threading::TimerThreadPool> timerThreadPool)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_address{address},
        m_username{username},
        m_password{password},
        m_socketThreadPool{socketThreadPool.Get()},
        m_timerThreadPool{timerThreadPool.Get()},
        m_registryClient{
          [=] (std::optional<
              Beam::RegistryService::ApplicationRegistryClient>& client) {
            client.emplace();
            client->BuildSession(Beam::Ref(*m_serviceLocatorClient),
              Beam::Ref(*m_socketThreadPool), Beam::Ref(*m_timerThreadPool));
            (*client)->Open();
          }
        },
        m_administrationClient{
          [=] (std::optional<
              AdministrationService::ApplicationAdministrationClient>& client) {
            client.emplace();
            client->BuildSession(Beam::Ref(*m_serviceLocatorClient),
              Beam::Ref(*m_socketThreadPool), Beam::Ref(*m_timerThreadPool));
            (*client)->Open();
          }
        },
        m_definitionsClient{
          [=] (std::optional<
              DefinitionsService::ApplicationDefinitionsClient>& client) {
            client.emplace();
            client->BuildSession(Beam::Ref(*m_serviceLocatorClient),
              Beam::Ref(*m_socketThreadPool), Beam::Ref(*m_timerThreadPool));
            (*client)->Open();
          }
        },
        m_marketDataClient{
          [=] (std::optional<
              MarketDataService::ApplicationMarketDataClient>& client) {
            client.emplace();
            client->BuildSession(Beam::Ref(*m_serviceLocatorClient),
              Beam::Ref(*m_socketThreadPool), Beam::Ref(*m_timerThreadPool));
            (*client)->Open();
          }
        },
        m_chartingClient{
          [=] (std::optional<
              ChartingService::ApplicationChartingClient>& client) {
            client.emplace();
            client->BuildSession(Beam::Ref(*m_serviceLocatorClient),
              Beam::Ref(*m_socketThreadPool), Beam::Ref(*m_timerThreadPool));
            (*client)->Open();
          }
        },
        m_complianceClient{
          [=] (std::optional<
              Compliance::ApplicationComplianceClient>& client) {
            client.emplace();
            client->BuildSession(Beam::Ref(*m_serviceLocatorClient),
              Beam::Ref(*m_socketThreadPool), Beam::Ref(*m_timerThreadPool));
            (*client)->Open();
          }
        },
        m_orderExecutionClient{
          [=] (std::optional<
              OrderExecutionService::ApplicationOrderExecutionClient>& client) {
            client.emplace();
            client->BuildSession(Beam::Ref(*m_serviceLocatorClient),
              Beam::Ref(*m_socketThreadPool), Beam::Ref(*m_timerThreadPool));
            (*client)->Open();
          }
        },
        m_riskClient{
          [=] (std::optional<RiskService::ApplicationRiskClient>& client) {
            client.emplace();
            client->BuildSession(Beam::Ref(*m_serviceLocatorClient),
              Beam::Ref(*m_socketThreadPool), Beam::Ref(*m_timerThreadPool));
            (*client)->Open();
          }
        },
        m_timeClient{
          [=] (std::optional<std::unique_ptr<TimeClient>>& client) {
            auto timeServices = m_serviceLocatorClient->Locate(
              Beam::TimeService::SERVICE_NAME);
            if(timeServices.empty()) {
              client.emplace(Beam::TimeService::MakeLiveNtpTimeClient({},
                Beam::Ref(*m_socketThreadPool), Beam::Ref(*m_timerThreadPool)));
              BOOST_THROW_EXCEPTION(
                Beam::IO::ConnectException{"No time services available."});
            }
            auto& timeService = timeServices.front();
            auto ntpPool = Beam::Parsers::Parse<
              std::vector<Beam::Network::IpAddress>>(boost::get<std::string>(
              timeService.GetProperties().At("addresses")));
            client.emplace(Beam::TimeService::MakeLiveNtpTimeClient(ntpPool,
              Beam::Ref(*m_socketThreadPool), Beam::Ref(*m_timerThreadPool)));
            try {
              (*client)->Open();
            } catch(const std::exception&) {
              BOOST_THROW_EXCEPTION(Beam::IO::ConnectException{
                "Unable to connect to the time service."});
            }
          }
        } {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

  inline ApplicationServiceClients::~ApplicationServiceClients() {
    Close();
  }

  inline ApplicationServiceClients::ServiceLocatorClient&
      ApplicationServiceClients::GetServiceLocatorClient() {
    return *m_serviceLocatorClient;
  }

  inline ApplicationServiceClients::RegistryClient&
      ApplicationServiceClients::GetRegistryClient() {
    return **m_registryClient;
  }

  inline ApplicationServiceClients::AdministrationClient&
      ApplicationServiceClients::GetAdministrationClient() {
    return **m_administrationClient;
  }

  inline ApplicationServiceClients::DefinitionsClient&
      ApplicationServiceClients::GetDefinitionsClient() {
    return **m_definitionsClient;
  }

  inline ApplicationServiceClients::MarketDataClient&
      ApplicationServiceClients::GetMarketDataClient() {
    return **m_marketDataClient;
  }

  inline ApplicationServiceClients::ChartingClient&
      ApplicationServiceClients::GetChartingClient() {
    return **m_chartingClient;
  }

  inline ApplicationServiceClients::ComplianceClient&
      ApplicationServiceClients::GetComplianceClient() {
    return **m_complianceClient;
  }

  inline ApplicationServiceClients::OrderExecutionClient&
      ApplicationServiceClients::GetOrderExecutionClient() {
    return **m_orderExecutionClient;
  }

  inline ApplicationServiceClients::RiskClient&
      ApplicationServiceClients::GetRiskClient() {
    return **m_riskClient;
  }

  inline ApplicationServiceClients::TimeClient&
      ApplicationServiceClients::GetTimeClient() {
    return **m_timeClient;
  }

  inline std::unique_ptr<ApplicationServiceClients::Timer>
      ApplicationServiceClients::BuildTimer(
      boost::posix_time::time_duration duration) {
    return std::make_unique<Timer>(duration, Beam::Ref(*m_timerThreadPool));
  }

  inline void ApplicationServiceClients::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_serviceLocatorClient.BuildSession(m_address,
        Beam::Ref(*m_socketThreadPool), Beam::Ref(*m_timerThreadPool));
      m_serviceLocatorClient->SetCredentials(m_username, m_password);
      m_serviceLocatorClient->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void ApplicationServiceClients::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void ApplicationServiceClients::Shutdown() {
    try {
      (*m_timeClient)->Close();
    } catch(const std::exception&) {}
    try {
      (*m_riskClient)->Close();
    } catch(const std::exception&) {}
    try {
      (*m_orderExecutionClient)->Close();
    } catch(const std::exception&) {}
    try {
      (*m_complianceClient)->Close();
    } catch(const std::exception&) {}
    try {
      (*m_chartingClient)->Close();
    } catch(const std::exception&) {}
    try {
      (*m_marketDataClient)->Close();
    } catch(const std::exception&) {}
    try {
      (*m_definitionsClient)->Close();
    } catch(const std::exception&) {}
    try {
      (*m_administrationClient)->Close();
    } catch(const std::exception&) {}
    try {
      (*m_registryClient)->Close();
    } catch(const std::exception&) {}
    try {
      m_serviceLocatorClient->Close();
    } catch(const std::exception&) {}
    m_openState.SetClosed();
  }
}

#endif
