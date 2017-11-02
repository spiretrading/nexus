#ifndef NEXUS_TO_PYTHON_SERVICE_CLIENTS_HPP
#define NEXUS_TO_PYTHON_SERVICE_CLIENTS_HPP
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/ToPythonServiceLocatorClient.hpp>
#include <Beam/Python/ToPythonTimeClient.hpp>
#include <Beam/Python/ToPythonTimer.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/Remote.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Python/ToPythonMarketDataClient.hpp"
#include "Nexus/Python/ToPythonOrderExecutionClient.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"

namespace Nexus {

  /*! \class ToPythonServiceClients
      \brief Wraps a ServiceClients instance for use within Python.
      \tparam ClientType The type of ServiceClients to wrap.
   */
  template<typename ClientType>
  class ToPythonServiceClients : public VirtualServiceClients {
    public:

      //! The type of ServiceClients to wrap.
      using Client = ClientType;

      //! Constructs a ToPythonServiceClients instance.
      /*!
        \param client The ServiceClients to wrap.
      */
      ToPythonServiceClients(std::unique_ptr<Client> client);

      virtual ~ToPythonServiceClients() override final;

      virtual ServiceLocatorClient& GetServiceLocatorClient() override final;

      virtual RegistryClient& GetRegistryClient() override final;

      virtual AdministrationClient& GetAdministrationClient() override final;

      virtual DefinitionsClient& GetDefinitionsClient() override final;

      virtual MarketDataClient& GetMarketDataClient() override final;

      virtual ChartingClient& GetChartingClient() override final;

      virtual ComplianceClient& GetComplianceClient() override final;

      virtual OrderExecutionClient& GetOrderExecutionClient() override final;

      virtual RiskClient& GetRiskClient() override final;

      virtual TimeClient& GetTimeClient() override final;

      virtual std::unique_ptr<Timer> BuildTimer(
        boost::posix_time::time_duration expiry);

      virtual void Open() override final;

      virtual void Close() override final;

    private:
      std::unique_ptr<Client> m_client;
      boost::optional<Beam::Remote<std::unique_ptr<ServiceLocatorClient>,
        Beam::Threading::Mutex>> m_serviceLocatorClient;
      boost::optional<Beam::Remote<std::unique_ptr<MarketDataClient>,
        Beam::Threading::Mutex>> m_marketDataClient;
      boost::optional<Beam::Remote<std::unique_ptr<OrderExecutionClient>,
        Beam::Threading::Mutex>> m_orderExecutionClient;
      boost::optional<Beam::Remote<std::unique_ptr<TimeClient>,
        Beam::Threading::Mutex>> m_timeClient;
  };

  //! Builds a ToPythonServiceClients instance.
  /*!
    \param client The ServiceClients instance to wrap.
  */
  template<typename Client>
  auto MakeToPythonServiceClients(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonServiceClients<Client>>(std::move(client));
  }

  template<typename ClientType>
  ToPythonServiceClients<ClientType>::ToPythonServiceClients(
      std::unique_ptr<Client> client)
      BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_client{std::move(client)},
        m_serviceLocatorClient{
          [=] (auto& client) {
            client.Initialize(
              Beam::ServiceLocator::MakeToPythonServiceLocatorClient(
              Beam::ServiceLocator::MakeVirtualServiceLocatorClient(
              &m_client->GetServiceLocatorClient())));
          }
        },
        m_marketDataClient{
          [=] (auto& client) {
            client.Initialize(
              MarketDataService::MakeToPythonMarketDataClient(
              MarketDataService::MakeVirtualMarketDataClient(
              &m_client->GetMarketDataClient())));
          }
        },
        m_orderExecutionClient{
          [=] (auto& client) {
            client.Initialize(
              OrderExecutionService::MakeToPythonOrderExecutionClient(
              OrderExecutionService::MakeVirtualOrderExecutionClient(
              &m_client->GetOrderExecutionClient())));
          }
        },
        m_timeClient{
          [=] (auto& client) {
            client.Initialize(Beam::TimeService::MakeToPythonTimeClient(
              Beam::TimeService::MakeVirtualTimeClient(
              &m_client->GetTimeClient())));
          }
        } {}
      BEAM_UNSUPPRESS_THIS_INITIALIZER()

  template<typename ClientType>
  ToPythonServiceClients<ClientType>::~ToPythonServiceClients() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_timeClient.reset();
    m_orderExecutionClient.reset();
    m_marketDataClient.reset();
    m_serviceLocatorClient.reset();
    m_client.reset();
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::ServiceLocatorClient&
      ToPythonServiceClients<ClientType>::GetServiceLocatorClient() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return ***m_serviceLocatorClient;
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::RegistryClient&
      ToPythonServiceClients<ClientType>::GetRegistryClient() {
    throw std::runtime_error{"Not implemented"};
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::AdministrationClient&
      ToPythonServiceClients<ClientType>::GetAdministrationClient() {
    throw std::runtime_error{"Not implemented"};
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::DefinitionsClient&
      ToPythonServiceClients<ClientType>::GetDefinitionsClient() {
    throw std::runtime_error{"Not implemented"};
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::MarketDataClient&
      ToPythonServiceClients<ClientType>::GetMarketDataClient() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return ***m_marketDataClient;
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::ChartingClient&
      ToPythonServiceClients<ClientType>::GetChartingClient() {
    throw std::runtime_error{"Not implemented"};
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::ComplianceClient&
      ToPythonServiceClients<ClientType>::GetComplianceClient() {
    throw std::runtime_error{"Not implemented"};
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::OrderExecutionClient&
      ToPythonServiceClients<ClientType>::GetOrderExecutionClient() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return ***m_orderExecutionClient;
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::RiskClient&
      ToPythonServiceClients<ClientType>::GetRiskClient() {
    throw std::runtime_error{"Not implemented"};
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::TimeClient&
      ToPythonServiceClients<ClientType>::GetTimeClient() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return ***m_timeClient;
  }

  template<typename ClientType>
  std::unique_ptr<typename ToPythonServiceClients<ClientType>::Timer>
      ToPythonServiceClients<ClientType>::BuildTimer(
      boost::posix_time::time_duration expiry) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return Beam::Threading::MakeToPythonTimer(m_client->BuildTimer(expiry));
  }

  template<typename ClientType>
  void ToPythonServiceClients<ClientType>::Open() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->Open();
  }

  template<typename ClientType>
  void ToPythonServiceClients<ClientType>::Close() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->Close();
  }
}

#endif
