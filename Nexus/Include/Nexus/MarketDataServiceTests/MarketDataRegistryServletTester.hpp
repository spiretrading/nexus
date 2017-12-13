#ifndef NEXUS_MARKETDATAREGISTRYSERVLETTESTER_HPP
#define NEXUS_MARKETDATAREGISTRYSERVLETTESTER_HPP
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTests.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Tests {

  /*  \class MarketDataRegistryServletTester
      \brief Tests the MarketDataRegistryServlet class.
   */
  class MarketDataRegistryServletTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServletContainer.
      using ServletContainer =
        Beam::Services::Tests::TestAuthenticatedServiceProtocolServletContainer<
        MetaMarketDataRegistryServlet<MarketDataRegistry*,
        LocalHistoricalDataStore,
        std::unique_ptr<AdministrationService::VirtualAdministrationClient>>,
        Beam::NativePointerPolicy>;

      virtual void setUp();

      virtual void tearDown();

      void TestMarketAndSourceEntitlement();

    private:
      boost::optional<
        Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment>
        m_serviceLocatorEnvironment;
      boost::optional<
        AdministrationService::Tests::AdministrationServiceTestEnvironment>
        m_administrationEnvironment;
      boost::optional<EntitlementDatabase> m_entitlements;
      boost::optional<MarketDataRegistry> m_registry;
      boost::optional<ServletContainer::Servlet::Servlet> m_registryServlet;
      boost::optional<ServletContainer> m_container;
      boost::optional<Beam::Services::Tests::TestServiceProtocolClient>
        m_clientProtocol;

      CPPUNIT_TEST_SUITE(MarketDataRegistryServletTester);
        CPPUNIT_TEST(TestMarketAndSourceEntitlement);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
