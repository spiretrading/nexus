#ifndef NEXUS_TESTSERVICECLIENTSINSTANCE_HPP
#define NEXUS_TESTSERVICECLIENTSINSTANCE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/UidServiceTests/UidServiceTestInstance.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestInstance.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceInstance.hpp"

namespace Nexus {

  /*! \class TestServiceClientsInstance
      \brief Stores test instances of Nexus services for the purpose of
             constructing test clients.
   */
  class TestServiceClientsInstance : private boost::noncopyable {
    public:
      void Open();

      void Close();

    private:
      Beam::ServiceLocator::Tests::ServiceLocatorTestInstance
        m_serviceLocatorInstance;
      Beam::UidService::Tests::UidServiceTestInstance m_uidInstance;
      boost::optional<
        AdministrationService::Tests::AdministrationServiceTestInstance>
        m_administrationInstance;
      boost::optional<
        OrderExecutionService::Tests::OrderExecutionServiceTestInstance>
        m_orderExecutionInstance;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline void TestServiceClientsInstance::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_serviceLocatorInstance.Open();
      m_uidInstance.Open();
      auto administrationServiceLocatorClient =
        m_serviceLocatorInstance.BuildClient();
      administrationServiceLocatorClient->SetCredentials("root", "");
      administrationServiceLocatorClient->Open();
      m_administrationInstance.emplace(
        std::move(administrationServiceLocatorClient));
      m_administrationInstance->Open();
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
    m_openState.SetClosed();
  }
}

#endif
