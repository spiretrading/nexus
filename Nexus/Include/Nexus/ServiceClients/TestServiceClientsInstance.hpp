#ifndef NEXUS_TESTSERVICECLIENTSINSTANCE_HPP
#define NEXUS_TESTSERVICECLIENTSINSTANCE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/UidServiceTests/UidServiceTestInstance.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestInstance.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestInstance.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceInstance.hpp"

namespace Nexus {

  /*! \class TestServiceClientsInstance
      \brief Stores test instances of Nexus services for the purpose of
             constructing test clients.
   */
  class TestServiceClientsInstance : private boost::noncopyable {
    public:
      ~TestServiceClientsInstance();

      //! Returns the ServiceLocatorTestInstance.
      Beam::ServiceLocator::Tests::ServiceLocatorTestInstance&
        GetServiceLocatorInstance();

      //! Returns the UidServiceTestInstance.
      Beam::UidService::Tests::UidServiceTestInstance& GetUidInstance();

      //! Returns the AdministrationServiceTestInstance.
      AdministrationService::Tests::AdministrationServiceTestInstance&
        GetAdministrationInstance();

      //! Returns the MarketDataServiceTestInstance.
      MarketDataService::Tests::MarketDataServiceTestInstance&
        GetMarketDataInstance();

      //! Returns the OrderExecutionServiceTestInstance.
      OrderExecutionService::Tests::OrderExecutionServiceTestInstance&
        GetOrderExecutionInstance();

      void Open();

      void Close();

    private:
      Beam::ServiceLocator::Tests::ServiceLocatorTestInstance
        m_serviceLocatorInstance;
      std::unique_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        m_serviceLocatorClient;
      Beam::UidService::Tests::UidServiceTestInstance m_uidInstance;
      boost::optional<
        AdministrationService::Tests::AdministrationServiceTestInstance>
        m_administrationInstance;
      boost::optional<MarketDataService::Tests::MarketDataServiceTestInstance>
        m_marketDataInstance;
      boost::optional<
        OrderExecutionService::Tests::OrderExecutionServiceTestInstance>
        m_orderExecutionInstance;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline TestServiceClientsInstance::~TestServiceClientsInstance() {
    Close();
  }

  inline Beam::ServiceLocator::Tests::ServiceLocatorTestInstance&
      TestServiceClientsInstance::GetServiceLocatorInstance() {
    return m_serviceLocatorInstance;
  }

  inline Beam::UidService::Tests::UidServiceTestInstance&
      TestServiceClientsInstance::GetUidInstance() {
    return m_uidInstance;
  }

  inline AdministrationService::Tests::AdministrationServiceTestInstance&
      TestServiceClientsInstance::GetAdministrationInstance() {
    return *m_administrationInstance;
  }

  inline MarketDataService::Tests::MarketDataServiceTestInstance&
      TestServiceClientsInstance::GetMarketDataInstance() {
    return *m_marketDataInstance;
  }

  inline OrderExecutionService::Tests::OrderExecutionServiceTestInstance&
      TestServiceClientsInstance::GetOrderExecutionInstance() {
    return *m_orderExecutionInstance;
  }

  inline void TestServiceClientsInstance::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_serviceLocatorInstance.Open();
      m_serviceLocatorClient = m_serviceLocatorInstance.BuildClient();
      m_serviceLocatorClient->SetCredentials("root", "");
      m_serviceLocatorClient->Open();
      m_uidInstance.Open();
      auto administrationServiceLocatorClient =
        m_serviceLocatorInstance.BuildClient();
      administrationServiceLocatorClient->SetCredentials("root", "");
      administrationServiceLocatorClient->Open();
      m_administrationInstance.emplace(
        std::move(administrationServiceLocatorClient));
      m_administrationInstance->Open();
      auto marketDataServiceLocatorClient =
        m_serviceLocatorInstance.BuildClient();
      marketDataServiceLocatorClient->SetCredentials("root", "");
      marketDataServiceLocatorClient->Open();
      m_marketDataInstance.emplace(std::move(marketDataServiceLocatorClient));
      auto orderExecutionServiceLocatorClient =
        m_serviceLocatorInstance.BuildClient();
      orderExecutionServiceLocatorClient->SetCredentials("root", "");
      orderExecutionServiceLocatorClient->Open();
      auto uidClient = m_uidInstance.BuildClient();
      uidClient->Open();
      auto administrationClient = m_administrationInstance->BuildClient(
        Beam::Ref(*m_serviceLocatorClient));
      administrationClient->Open();
      m_orderExecutionInstance.emplace(
        std::move(orderExecutionServiceLocatorClient),
        std::move(uidClient), std::move(administrationClient));
      m_orderExecutionInstance->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void TestServiceClientsInstance::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void TestServiceClientsInstance::Shutdown() {
    m_orderExecutionInstance.reset();
    m_marketDataInstance.reset();
    m_administrationInstance.reset();
    m_uidInstance.Close();
    m_serviceLocatorClient.reset();
    m_serviceLocatorInstance.Close();
    m_openState.SetClosed();
  }
}

#endif
