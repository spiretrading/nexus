#ifndef NEXUS_BUYINGPOWERCOMPLIANCERULETESTER_HPP
#define NEXUS_BUYINGPOWERCOMPLIANCERULETESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/Compliance/BuyingPowerComplianceRule.hpp"
#include "Nexus/ComplianceTests/ComplianceTests.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class BuyingPowerComplianceRuleTester
      \brief Tests the BuyingPowerComplianceRule class.
   */
  class BuyingPowerComplianceRuleTester :
      public CPPUNIT_NS::TestFixture {
    public:

      //! The type of compliance rule to test.
      using BuyingPowerComplianceRule =
        Compliance::BuyingPowerComplianceRule<
        MarketDataService::VirtualMarketDataClient*>;

      virtual void setUp();

      virtual void tearDown();

      //! Add a filled order for 100 shares at $1.00.
      //! Submit an order for 100 shares at $2.00.
      //! Expect the order to be accepted.
      void TestOrderRecovery();

    private:
      boost::optional<TestEnvironment> m_testEnvironment;
      std::shared_ptr<Beam::Queue<const OrderExecutionService::Order*>>
        m_orderSubmissions;
      boost::optional<TestServiceClients> m_serviceClients;

      CPPUNIT_TEST_SUITE(BuyingPowerComplianceRuleTester);
        CPPUNIT_TEST(TestOrderRecovery);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
