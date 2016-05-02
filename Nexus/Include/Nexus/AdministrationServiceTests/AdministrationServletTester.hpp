#ifndef NEXUS_ADMINISTRATIONSERVLETTESTER_HPP
#define NEXUS_ADMINISTRATIONSERVLETTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTests.hpp"

namespace Nexus {
namespace AdministrationService {
namespace Tests {

  /*! \class AdministrationServletTester
      \brief Tests the AdministrationServlet class.
   */
  class AdministrationServletTester: public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServerConnection.
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServletContainer = Beam::Services::ServiceProtocolServletContainer<
        MetaAdministrationServlet<
        Beam::ServiceLocator::VirtualServiceLocatorClient*,
        std::shared_ptr<LocalAdministrationDataStore>>,
        std::shared_ptr<ServerConnection>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      //! The type of ServiceProtocol on the client side.
      using ClientServiceProtocolClient =
        Beam::Services::ServiceProtocolClient<
        Beam::Services::MessageProtocol<ClientChannel,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>>,
        Beam::Threading::TriggerTimer>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests loading an account's identity.
      void TestLoadAccountIdentity();

      //! Tests loading the identity of an account that doesn't exist.
      void TestInvalidLoadAccountIdentity();

      //! Tests setting an account's identity for the first time.
      void TestInitialSetAccountIdentity();

      //! Tests setting an existing account's identity.
      void TestExistingSetAccountIdentity();

      //! Tests setting the identity of an account that doesn't exist.
      void TestInvalidSetAccountIdentity();

      //! Tests loading the list of administrators.
      void TestLoadAdministrators();

      //! Tests loading the list of services.
      void TestLoadServices();

      //! Tests loading the list of managed trading groups.
      void TestLoadManagedTradingGroups();

    private:
      Beam::DelayPtr<Beam::ServiceLocator::Tests::ServiceLocatorTestInstance>
        m_serviceLocatorInstance;
      std::shared_ptr<LocalAdministrationDataStore> m_dataStore;
      std::shared_ptr<ServerConnection> m_serverConnection;
      Beam::DelayPtr<ServletContainer> m_container;
      Beam::DelayPtr<ClientServiceProtocolClient> m_clientProtocol;

      Beam::ServiceLocator::DirectoryEntry GetAdministratorsDirectory();
      Beam::ServiceLocator::DirectoryEntry GetServicesDirectory();
      Beam::ServiceLocator::DirectoryEntry GetTradingGroupsDirectory();
      Beam::ServiceLocator::DirectoryEntry MakeAccount(
        const std::string& name,
        const Beam::ServiceLocator::DirectoryEntry& parent);
      Beam::ServiceLocator::DirectoryEntry MakeTradingGroup(
        const std::string& name);

      CPPUNIT_TEST_SUITE(AdministrationServletTester);
        CPPUNIT_TEST(TestLoadAccountIdentity);
        CPPUNIT_TEST(TestInvalidLoadAccountIdentity);
        CPPUNIT_TEST(TestInitialSetAccountIdentity);
        CPPUNIT_TEST(TestExistingSetAccountIdentity);
        CPPUNIT_TEST(TestInvalidSetAccountIdentity);
        CPPUNIT_TEST(TestLoadAdministrators);
        CPPUNIT_TEST(TestLoadServices);
        CPPUNIT_TEST(TestLoadManagedTradingGroups);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
