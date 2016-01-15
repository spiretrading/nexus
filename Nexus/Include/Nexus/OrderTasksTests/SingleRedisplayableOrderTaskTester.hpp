#ifndef NEXUS_SINGLEREDISPLAYABLEORDERTASKTESTER_HPP
#define NEXUS_SINGLEREDISPLAYABLEORDERTASKTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/SignalHandling/SignalSink.hpp>
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionClient.hpp"
#include "Nexus/OrderTasks/SingleRedisplayableOrderTask.hpp"

namespace Nexus {
namespace OrderTasks {
namespace Tests {

  /*! \class SingleRedisplayableOrderTaskTester
      \brief Tests the SingleOrderTask class.
   */
  class SingleRedisplayableOrderTaskTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of OrderTaskFactory.
      typedef SingleOrderTaskFactory<
        OrderExecutionService::Tests::MockOrderExecutionClient>
        TestSingleOrderTaskFactory;

      virtual void setUp();
      virtual void tearDown();

      //! Tests executing an Order with an initial size of 0.
      void TestZeroSizedInitialOrder();

      //! Tests executing an Order with an initial size of 100.
      void TestNonEmptyInitialOrder();

      //! Tests redisplaying from 0 to 100.
      void TestResizingFromEmptyOrder();

      //! Tests trivially redisplaying an order to its existing size.
      void TestResizingToExistingSize();

      //! Tests increasing a non-empty order.
      void TestIncreasingNonEmptyOrder();

      //! Tests decreasing an order to 0.
      void TestDecreasingToEmptyOrder();

      //! Tests decreasing an order to a non-empty order.
      void TestDecreasingToNonEmptyOrder();

      //! Tests filling an order and then decreasing it to an empty order.
      void TestDecreasingFilledOrderToEmptyOrder();

      //! Tests filling an order and then decreasing it.
      void TestDecreasingFilledOrder();

      //! Tests filling an order completely and then increasing it.
      void TestIncreasingFilledOrder();

      //! Tests filling an order partially and then increasing it.
      void TestIncreasingPartiallyFilledOrder();

      //! Tests an underlying order that EXPIREs.
      void TestExpiredOrder();

      //! Tests an underlying order that EXPIREs while being redisplayed.
      void TestPreemptiveExpiredOrder();

      //! Tests an underlying order that COMPLETEs.
      void TestCompletedOrder();

      //! Tests an underlying order that COMPLETEs while being redisplayed.
      void TestPreemptiveCompletedOrder();

      //! Tests an underlying order that's FULL.
      void TestFullOrder();

      //! Tests the continuation of an order with no fill.
      void TestUnfilledContinuation();

      //! Tests the continuation of an order that's been partially filled.
      void TestPartiallyFilledContinuation();

    private:
      Beam::DelayPtr<OrderExecutionService::Tests::MockOrderExecutionClient>
        m_client;
      Beam::DelayPtr<Beam::SignalHandling::SignalSink> m_orderSink;
      Beam::DelayPtr<Beam::SignalHandling::SignalSink> m_executionSink;

      CPPUNIT_TEST_SUITE(SingleRedisplayableOrderTaskTester);
        CPPUNIT_TEST(TestZeroSizedInitialOrder);
        CPPUNIT_TEST(TestNonEmptyInitialOrder);
        CPPUNIT_TEST(TestResizingFromEmptyOrder);
        CPPUNIT_TEST(TestResizingToExistingSize);
        CPPUNIT_TEST(TestIncreasingNonEmptyOrder);
        CPPUNIT_TEST(TestDecreasingToEmptyOrder);
        CPPUNIT_TEST(TestDecreasingToNonEmptyOrder);
        CPPUNIT_TEST(TestDecreasingFilledOrderToEmptyOrder);
        CPPUNIT_TEST(TestDecreasingFilledOrder);
        CPPUNIT_TEST(TestIncreasingFilledOrder);
        CPPUNIT_TEST(TestIncreasingPartiallyFilledOrder);
        CPPUNIT_TEST(TestExpiredOrder);
        CPPUNIT_TEST(TestPreemptiveExpiredOrder);
        CPPUNIT_TEST(TestCompletedOrder);
        CPPUNIT_TEST(TestPreemptiveCompletedOrder);
        CPPUNIT_TEST(TestFullOrder);
        CPPUNIT_TEST(TestUnfilledContinuation);
        CPPUNIT_TEST(TestPartiallyFilledContinuation);
      CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
