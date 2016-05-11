#ifndef NEXUS_INTERNALMATCHINGORDEREXECUTIONDRIVERTESTER_HPP
#define NEXUS_INTERNALMATCHINGORDEREXECUTIONDRIVERTESTER_HPP
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/Threading/TimerThreadPool.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestInstance.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/InternalMatcher/InternalMatchingOrderExecutionDriver.hpp"
#include "Nexus/InternalMatcher/NullMatchReportBuilder.hpp"
#include "Nexus/InternalMatcherTests/InternalMatcherTests.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestInstance.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"

namespace Nexus {
namespace InternalMatcher {
namespace Tests {

  /*! \class InternalMatchingOrderExecutionDriverTester
      \brief Tests the InternalMatchingOrderExecutionDriver.
   */
  class InternalMatchingOrderExecutionDriverTester :
      public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceLocatorClient.
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      //! The type of MarketDataClient.
      using MarketDataClient = MarketDataService::VirtualMarketDataClient;

      //! The type of UidClient.
      using UidClient = Beam::UidService::VirtualUidClient;

      //! The type of driver to test.
      using TestInternalMatchingOrderExecutionDriver =
        InternalMatchingOrderExecutionDriver<NullMatchReportBuilder,
        std::unique_ptr<MarketDataClient>,
        Beam::TimeService::IncrementalTimeClient, std::unique_ptr<UidClient>,
        OrderExecutionService::Tests::MockOrderExecutionDriver*>;

      virtual void setUp();

      virtual void tearDown();

      //! Set the BBO to 1.00/1.01.
      //! Execute a bid for 100 shares at 1.00.
      //! Execute an ask for 100 shares at 1.01.
      //! Cancel the ask.
      //! Cancel the bid.
      void TestBidAndAskWithoutMatching();

      //! Set the BBO to 1.00/1.01.
      //! Execute a bid for 100 shares at 1.00.
      //! Submit an ask for 100 shares at 1.00.
      //! Expect the bid to get passively matched for 100 shares at 1.00.
      //! Expect the ask to transition to state NEW.
      //! Expect the ask to get actively matched for 100 shares at 1.00.
      void TestAddBidAndRemoveAskWithMatching();

      //! Set the BBO to 1.00/1.01.
      //! Execute an ask for 100 shares at 1.01.
      //! Submit a bid for 100 shares at 1.01.
      //! Expect the ask to get passively matched for 100 shares at 1.01.
      //! Expect the bid to transition to state NEW.
      //! Expect the bid to get passively matched for 100 shares at 1.01.
      void TestAddAskAndRemoveBidWithMatching();

      //! Set the BBO to 1.00/1.01.
      //! Execute a bid A for 100 shares at 1.00.
      //! Execute a bid B for 100 shares at 1.00.
      //! Submit an ask for 200 shares at 1.00.
      //! Expect bid A to get passively matched for 100 shares at 1.00.
      //! Expect bid B to get passively matched for 100 shares at 1.00.
      //! Expect the ask to transition to state NEW.
      //! Expect the ask to get actively matched for 100 shares at 1.00.
      //! Expect the ask to get actively matched for 100 shares at 1.00.
      void TestAddMultipleBidsAndRemoveAskWithMatching();

      //! Set the BBO to 1.00/1.01.
      //! Execute a bid for 100 shares at 0.50.
      //! Execute an ask for 100 shares at 0.01.
      //! Cancel the ask.
      //! Cancel the bid.
      void TestFarBidAndCrossedAskWithoutMatching();

      //! Set the BBO to 1.00/1.01.
      //! Execute an ask for 100 shares at 2.00.
      //! Execute a bid for 100 shares at 3.00.
      //! Cancel the bid.
      //! Cancel the ask.
      void TestFarAskAndCrossedBidWithoutMatching();

      //! Set the BBO to 1.00/1.01.
      //! Execute a bid for 1000 shares at 1.00.
      //! Submit an ask for 100 shares at 1.00.
      //! Expect the bid to passively match for 100 shares at 1.00.
      //! Expect the ask to transition to state new.
      //! Expect the ask to actively match for 100 shares at 1.00.
      //! Cancel the bid.
      void TestAddBidAndPartialRemoveAskWithMatching();

      //! Set the BBO to 1.00/1.01.
      //! Execute an ask for 1000 shares at 1.01.
      //! Submit a bid for 100 shares at 1.01.
      //! Expect the ask to passively match for 100 shares at 1.01.
      //! Expect the bid to transition to state new.
      //! Expect the bid to actively match for 100 shares at 1.01.
      //! Cancel the ask.
      void TestAddAskAndPartialRemoveBidWithMatching();

      //! Set the BBO to 1.00/1.01.
      //! Execute a bid for 100 shares at 1.00.
      //! Submit an ask for 1000 shares at 1.00.
      //! Expect the bid to passively match for 100 shares at 1.00.
      //! Expect the ask to transition to state new.
      //! Expect the ask to passively match for 100 shares at 1.00.
      //! Cancel the ask.
      void TestAddBidAndRemoveAskWithMatchingLeftOver();

      //! Set the BBO to 1.00/1.01.
      //! Execute a bid for 100 shares at 1.00.
      //! Submit an ask for 100 shares at 1.00.
      //! Expect the bid to enter a PENDING_CANCEL state.
      //! Fill the bid for 100 shares at 1.00.
      //! Accept the ask.
      //! Expect the ask to transition to state new.
      //! Cancel the ask.
      void TestAddBidAndRemoveAskWithPrematureFill();

      //! Set the BBO to 1.00/1.01.
      //! Execute a bid for 1000 shares at 1.00.
      //! Submit an ask for 100 shares at 1.00.
      //! Expect the bid to enter a PENDING_CANCEL state.
      //! Fill the bid for 100 shares.
      //! Expect the bid to get passively matched for 100 shares.
      //! Expect the ask to transition to state new.
      //! Expect the ask to get actively matched for 100 shares.
      //! Cancel the bid.
      void TestAddBidAndRemoveAskWithPrematurePartialFill();

    private:
      struct OrderEntry {
        OrderExecutionService::OrderFields m_fields;
        std::shared_ptr<Beam::Queue<OrderExecutionService::ExecutionReport>>
          m_executionReportQueue;
        const OrderExecutionService::Order* m_order;
        std::shared_ptr<Beam::Queue<OrderExecutionService::ExecutionReport>>
          m_mockExecutionReportQueue;
        OrderExecutionService::PrimitiveOrder* m_mockOrder;
        Quantity m_remainingQuantity;

        OrderEntry();
      };
      Beam::DelayPtr<Beam::Threading::TimerThreadPool> m_timerThreadPool;
      Beam::DelayPtr<Beam::ServiceLocator::Tests::ServiceLocatorTestInstance>
        m_serviceLocatorInstance;
      std::unique_ptr<ServiceLocatorClient> m_serviceLocatorClient;
      Beam::DelayPtr<MarketDataService::Tests::MarketDataServiceTestInstance>
        m_marketDataServiceInstance;
      Beam::DelayPtr<Beam::UidService::Tests::UidServiceTestInstance>
        m_uidServiceInstance;
      Beam::DelayPtr<OrderExecutionService::Tests::MockOrderExecutionDriver>
        m_mockOrderExecutionDriver;
      std::shared_ptr<Beam::Queue<OrderExecutionService::PrimitiveOrder*>>
        m_mockDriverMonitor;
      std::unique_ptr<UidClient> m_uidClient;
      Beam::DelayPtr<TestInternalMatchingOrderExecutionDriver>
        m_orderExecutionDriver;
      std::unique_ptr<ServiceLocatorClient>
        m_marketDataFeedServiceLocatorClient;

      void SetBbo(Money bid, Money ask);
      OrderEntry Submit(Side side, Money price, Quantity quantity);
      void Accept(OrderEntry& orderEntry);
      OrderEntry Execute(Side side, Money price, Quantity quantity);
      void Fill(OrderEntry& orderEntry, Money price, Quantity quantity);
      void Cancel(OrderEntry& orderEntry);
      void ExpectStatus(const std::shared_ptr<
        Beam::Queue<OrderExecutionService::ExecutionReport>>& queue,
        OrderStatus status);
      void ExpectPassiveInternalMatch(OrderEntry& orderEntry,
        Quantity expectedQuantity, int condition = 0);
      void ExpectActiveInternalMatch(OrderEntry& orderEntry,
        Money expectedPrice, Quantity expectedQuantity, int condition = 0);
      void PullMockOrder(std::shared_ptr<
        Beam::Queue<OrderExecutionService::PrimitiveOrder*>>& monitor,
        Beam::Out<OrderEntry> orderEntry);

      CPPUNIT_TEST_SUITE(InternalMatchingOrderExecutionDriverTester);
        CPPUNIT_TEST(TestBidAndAskWithoutMatching);
        CPPUNIT_TEST(TestAddBidAndRemoveAskWithMatching);
        CPPUNIT_TEST(TestAddAskAndRemoveBidWithMatching);
        CPPUNIT_TEST(TestAddMultipleBidsAndRemoveAskWithMatching);
        CPPUNIT_TEST(TestFarBidAndCrossedAskWithoutMatching);
        CPPUNIT_TEST(TestFarAskAndCrossedBidWithoutMatching);
        CPPUNIT_TEST(TestAddBidAndPartialRemoveAskWithMatching);
        CPPUNIT_TEST(TestAddAskAndPartialRemoveBidWithMatching);
        CPPUNIT_TEST(TestAddBidAndRemoveAskWithMatchingLeftOver);
        CPPUNIT_TEST(TestAddBidAndRemoveAskWithPrematureFill);
        CPPUNIT_TEST(TestAddBidAndRemoveAskWithPrematurePartialFill);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
