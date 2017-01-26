#ifndef NEXUS_TESTSERVICECLIENTS_HPP
#define NEXUS_TESTSERVICECLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestInstance.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestInstance.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceInstance.hpp"

namespace Nexus {

  /*! \class TestServiceClientsInstance
      \brief Implements the ServiceClients interface for testing.
   */
  class TestServiceClientsInstance : private boost::noncopyable {
    public:

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
        Beam::RefType<Beam::Network::SocketThreadPool> socketThreadPool,
        Beam::RefType<Beam::Threading::TimerThreadPool> timerThreadPool);

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

      void Open();

      void Close();
  };

  /*! \class TestServiceClients
      \brief Implements the ServiceClients interface for testing.
   */
  class TestServiceClients : private boost::noncopyable {
    public:

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
        Beam::RefType<Beam::Network::SocketThreadPool> socketThreadPool,
        Beam::RefType<Beam::Threading::TimerThreadPool> timerThreadPool);

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

      void Open();

      void Close();
  };
}

#endif
