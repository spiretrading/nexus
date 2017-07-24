#ifndef NEXUS_RISKSERVLETTESTER_HPP
#define NEXUS_RISKSERVLETTESTER_HPP
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/RiskService/RiskServlet.hpp"
#include "Nexus/RiskService/RiskStateMonitor.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

namespace Nexus {
namespace RiskService {
namespace Tests {

  /*! \class RiskServletTester
      \brief Tests the RiskServlet class.
   */
  class RiskServletTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of Portfolio used.
      using Portfolio =
        Accounting::Portfolio<Accounting::TrueAverageBookkeeper<
        Accounting::Inventory<Accounting::Position<Security>>>>;

      using TestRiskStateMonitor = RiskStateMonitor<RiskStateTracker<Portfolio,
        Beam::TimeService::VirtualTimeClient*>,
        AdministrationService::VirtualAdministrationClient*,
        MarketDataService::VirtualMarketDataClient*,
        Beam::Threading::VirtualTimer*, Beam::TimeService::VirtualTimeClient*>;

      //! The type of ServiceProtocolServer.
      using ServletContainer =
        Beam::Services::Tests::TestAuthenticatedServiceProtocolServletContainer<
        MetaRiskServlet<AdministrationService::VirtualAdministrationClient*,
        OrderExecutionService::VirtualOrderExecutionClient*,
        TestRiskStateMonitor>>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests opening a position within an account's RiskParameters.
      void TestOpenPosition();

    private:
      boost::optional<TestEnvironment> m_environment;
      boost::optional<TestServiceClients> m_serviceClients;
      boost::optional<ServletContainer> m_container;
      std::shared_ptr<Beam::Queue<const OrderExecutionService::Order*>>
        m_orderSubmissionServletQueue;
      std::shared_ptr<Beam::Queue<const OrderExecutionService::Order*>>
        m_orderSubmissionTransitionQueue;

      CPPUNIT_TEST_SUITE(RiskServletTester);
        CPPUNIT_TEST(TestOpenPosition);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
