#ifndef NEXUS_COMPLIANCERULESETTESTER_HPP
#define NEXUS_COMPLIANCERULESETTESTER_HPP
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceRuleSet.hpp"
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class ComplianceRuleSetTester
      \brief Tests the ComplianceRuleSet class.
   */
  class ComplianceRuleSetTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ComplianceClient.
      using TestComplianceClient = ComplianceClient<
        Beam::Services::Tests::TestServiceProtocolClientBuilder>;

      //! The type of ComplianceRuleSet to test.
      using TestComplianceRuleSet = ComplianceRuleSet<TestComplianceClient*,
        std::unique_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests submitting an Order.
      void TestSubmit();

    private:
      boost::optional<
        Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment>
        m_serviceLocatorEnvironment;
      boost::optional<Beam::Services::Tests::TestServiceProtocolServer>
        m_server;
      boost::optional<TestComplianceClient> m_complianceClient;
      boost::optional<TestComplianceRuleSet> m_complianceRuleSet;

      CPPUNIT_TEST_SUITE(ComplianceRuleSetTester);
        CPPUNIT_TEST(TestSubmit);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
