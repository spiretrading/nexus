#ifndef NEXUS_ADMINISTRATIONSERVLETTESTER_HPP
#define NEXUS_ADMINISTRATIONSERVLETTESTER_HPP
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
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

      //! The type of ServiceProtocolServer.
      using ServletContainer =
        Beam::Services::Tests::TestServiceProtocolServletContainer<
        MetaAdministrationServlet<
        Beam::ServiceLocator::VirtualServiceLocatorClient*,
        std::shared_ptr<LocalAdministrationDataStore>>>;

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
      boost::optional<
        Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment>
        m_serviceLocatorEnvironment;
      std::shared_ptr<LocalAdministrationDataStore> m_dataStore;
      boost::optional<ServletContainer> m_container;
      boost::optional<Beam::Services::Tests::TestServiceProtocolClient>
        m_clientProtocol;

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
