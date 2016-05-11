#ifndef NEXUS_RISKSERVLETTESTER_HPP
#define NEXUS_RISKSERVLETTESTER_HPP
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestInstance.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestInstance.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestInstance.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceInstance.hpp"
#include "Nexus/RiskService/RiskServlet.hpp"
#include "Nexus/RiskService/RiskStateMonitor.hpp"

namespace Nexus {
namespace RiskService {
namespace Tests {

  /*! \class RiskServletTester
      \brief Tests the RiskServlet class.
   */
  class RiskServletTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceLocatorClient.
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      //! The type of AdministrationClient.
      using AdministrationClient =
        AdministrationService::VirtualAdministrationClient;

      //! The type of MarketDataClient.
      using MarketDataClient = MarketDataService::VirtualMarketDataClient;

      //! The type of OrderExecutionClient.
      using OrderExecutionClient =
        OrderExecutionService::VirtualOrderExecutionClient;

      //! The type of Portfolio used.
      using Portfolio =
        Accounting::Portfolio<Accounting::TrueAverageBookkeeper<
        Accounting::Inventory<Accounting::Position<Security>>>>;

      using TestRiskStateMonitor = RiskStateMonitor<RiskStateTracker<Portfolio,
        Beam::TimeService::IncrementalTimeClient*>, AdministrationClient*,
        MarketDataClient*, Beam::Threading::TriggerTimer*,
        Beam::TimeService::IncrementalTimeClient>;

      //! The type of ServerConnection.
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServletContainer = Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaRiskServlet<std::unique_ptr<AdministrationClient>,
        std::shared_ptr<OrderExecutionClient>, TestRiskStateMonitor>,
        std::unique_ptr<ServiceLocatorClient>>, ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests opening a position within an account's RiskParameters.
      void TestOpenPosition();

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
      std::unique_ptr<MarketDataClient> m_marketDataClient;
      Beam::DelayPtr<OrderExecutionService::Tests::
        OrderExecutionServiceTestInstance> m_orderExecutionServiceInstance;
      Beam::DelayPtr<Beam::Threading::TriggerTimer> m_transitionTimer;
      Beam::DelayPtr<ServerConnection> m_serverConnection;
      Beam::DelayPtr<ServletContainer> m_container;
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
