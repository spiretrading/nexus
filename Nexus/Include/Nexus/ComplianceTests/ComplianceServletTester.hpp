#ifndef NEXUS_COMPLIANCESERVLETTESTER_HPP
#define NEXUS_COMPLIANCESERVLETTESTER_HPP
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Compliance/ComplianceRuleSet.hpp"
#include "Nexus/Compliance/ComplianceServlet.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class ComplianceServletTester
      \brief Tests the ComplianceServlet class.
   */
  class ComplianceServletTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ComplianceRuleSet to test.
      using TestComplianceRuleSet = ComplianceRuleSet<
        LocalComplianceRuleDataStore*,
        std::unique_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>>;

      //! The type of ServiceProtocolServer.
      using ServletContainer =
        Beam::Services::Tests::TestAuthenticatedServiceProtocolServletContainer<
        MetaComplianceServlet<
        std::unique_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>,
        std::unique_ptr<AdministrationService::VirtualAdministrationClient>,
        TestComplianceRuleSet*, Beam::TimeService::IncrementalTimeClient>>;

      virtual void setUp();

      virtual void tearDown();

    private:
      boost::optional<
        Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment>
        m_serviceLocatorEnvironment;
      boost::optional<
        AdministrationService::Tests::AdministrationServiceTestEnvironment>
        m_administrationServiceEnvironment;
      boost::optional<LocalComplianceRuleDataStore> m_dataStore;
      boost::optional<TestComplianceRuleSet> m_complianceRuleSet;
      boost::optional<ServletContainer> m_container;

      CPPUNIT_TEST_SUITE(ComplianceServletTester);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
