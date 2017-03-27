#ifndef NEXUS_BUYINGPOWERCHECKTESTER_HPP
#define NEXUS_BUYINGPOWERCHECKTESTER_HPP
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionService/BuyingPowerCheck.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Tests {

  /*! \class BuyingPowerCheckTester
      \brief Tests the BuyingPowerCheck class.
   */
  class BuyingPowerCheckTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceLocatorClient.
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      //! The type of AdministrationClient.
      using AdministrationClient =
        AdministrationService::VirtualAdministrationClient;

      //! The type of MarketDataClient.
      using MarketDataClient = MarketDataService::VirtualMarketDataClient;

      //! The type of BuyingPowerCheck to test.
      using BuyingPowerCheck = OrderExecutionService::BuyingPowerCheck<
        std::unique_ptr<AdministrationClient>,
        std::unique_ptr<MarketDataClient>>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests a series of submissions.
      void TestSubmission();

      //! Tests adding an Order without a previous submission.
      void TestAddWithoutSubmission();

      //! Tests a submission that is then rejected.
      void TestSubmissionThenRejection();

    private:
      Beam::DelayPtr<Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment>
        m_serviceLocatorEnvironment;
      Beam::DelayPtr<Beam::UidService::Tests::UidServiceTestEnvironment>
        m_uidServiceEnvironment;
      Beam::DelayPtr<
        AdministrationService::Tests::AdministrationServiceTestEnvironment>
        m_administrationServiceEnvironment;
      Beam::DelayPtr<MarketDataService::Tests::MarketDataServiceTestEnvironment>
        m_marketDataServiceEnvironment;
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
