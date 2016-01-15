#ifndef NEXUS_BUYINGPOWERCHECKTESTER_HPP
#define NEXUS_BUYINGPOWERCHECKTESTER_HPP
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestInstance.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestInstance.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestInstance.hpp"
#include "Nexus/OrderExecutionService/BuyingPowerCheck.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceInstance.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Tests {

  /*! \class BuyingPowerCheckTester
      \brief Tests the BuyingPowerCheck class.
   */
  class BuyingPowerCheckTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceLocatorClient.
      typedef Beam::ServiceLocator::Tests::ServiceLocatorTestInstance::
        ServiceLocatorClient ServiceLocatorClient;

      //! The type of AdministrationClient.
      typedef AdministrationService::Tests::AdministrationServiceTestInstance::
        AdministrationClient AdministrationClient;

      //! The type of MarketDataClient.
      typedef MarketDataService::Tests::MarketDataServiceTestInstance::
        MarketDataClient MarketDataClient;

      //! The type of BuyingPowerCheck to test.
      typedef OrderExecutionService::BuyingPowerCheck<
        std::unique_ptr<AdministrationClient>,
        std::unique_ptr<MarketDataClient>> BuyingPowerCheck;

      virtual void setUp();

      virtual void tearDown();

      //! Tests a series of submissions.
      void TestSubmission();

      //! Tests adding an Order without a previous submission.
      void TestAddWithoutSubmission();

      //! Tests a submission that is then rejected.
      void TestSubmissionThenRejection();

    private:
      Beam::DelayPtr<Beam::ServiceLocator::Tests::ServiceLocatorTestInstance>
        m_serviceLocatorInstance;
      Beam::DelayPtr<Beam::UidService::Tests::UidServiceTestInstance>
        m_uidServiceInstance;
      Beam::DelayPtr<
        AdministrationService::Tests::AdministrationServiceTestInstance>
        m_administrationServiceInstance;
      Beam::DelayPtr<MarketDataService::Tests::MarketDataServiceTestInstance>
        m_marketDataServiceInstance;
      std::unique_ptr<ServiceLocatorClient> m_serviceLocatorClient;
      Beam::DelayPtr<BuyingPowerCheck> m_buyingPowerCheck;
      Beam::ServiceLocator::DirectoryEntry m_traderAccount;
      RiskService::RiskParameters m_traderRiskParameters;

      CPPUNIT_TEST_SUITE(BuyingPowerCheckTester);
        CPPUNIT_TEST(TestSubmission);
        CPPUNIT_TEST(TestAddWithoutSubmission);
        CPPUNIT_TEST(TestSubmissionThenRejection);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
