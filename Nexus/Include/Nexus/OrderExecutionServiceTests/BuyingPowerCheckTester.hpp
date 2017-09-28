#ifndef NEXUS_BUYINGPOWERCHECKTESTER_HPP
#define NEXUS_BUYINGPOWERCHECKTESTER_HPP
#include <Beam/Queues/Queue.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/OrderExecutionService/BuyingPowerCheck.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Tests {

  /*! \class BuyingPowerCheckTester
      \brief Tests the BuyingPowerCheck class.
   */
  class BuyingPowerCheckTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of BuyingPowerCheck to test.
      using BuyingPowerCheck = OrderExecutionService::BuyingPowerCheck<
        AdministrationService::VirtualAdministrationClient*,
        MarketDataService::VirtualMarketDataClient*>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests a series of submissions.
      void TestSubmission();

      //! Tests adding an Order without a previous submission.
      void TestAddWithoutSubmission();

      //! Tests a submission that is then rejected.
      void TestSubmissionThenRejection();

      //! Add a filled order for 100 shares at $1.00.
      //! Submit an order for 100 shares at $2.00.
      //! Expect the order to be accepted.
      void TestOrderRecovery();

    private:
      boost::optional<TestEnvironment> m_environment;
      std::shared_ptr<Beam::Queue<const OrderExecutionService::Order*>>
        m_orderSubmissions;
      boost::optional<TestServiceClients> m_serviceClients;
      boost::optional<BuyingPowerCheck> m_buyingPowerCheck;
      RiskService::RiskParameters m_traderRiskParameters;

      CPPUNIT_TEST_SUITE(BuyingPowerCheckTester);
        CPPUNIT_TEST(TestSubmission);
        CPPUNIT_TEST(TestAddWithoutSubmission);
        CPPUNIT_TEST(TestSubmissionThenRejection);
        CPPUNIT_TEST(TestOrderRecovery);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
