#ifndef NEXUS_SINGLEORDERTASKTESTER_HPP
#define NEXUS_SINGLEORDERTASKTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/SignalHandling/SignalSink.hpp>
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionClient.hpp"
#include "Nexus/OrderTasks/SingleOrderTask.hpp"

namespace Nexus {
namespace OrderTasks {
namespace Tests {

  /*! \class SingleOrderTaskTester
      \brief Tests the SingleOrderTask class.
   */
  class SingleOrderTaskTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of OrderTaskFactory.
      typedef SingleOrderTaskFactory<
        OrderExecutionService::Tests::MockOrderExecutionClient>
        TestSingleOrderTaskFactory;

      virtual void setUp();
      virtual void tearDown();

      //! Tests executing a SingleOrderTask with a size of 0.
      void TestEmptyOrder();

      //! Tests a rejection.
      void TestRejectedOrder();

      //! Tests filling a SingleOrderTask.
      void TestFullOrder();

      //! Tests cancelling before the Order goes PENDING.
      void TestCancelPriorToPendingOrder();

      //! Tests a pre-emptive fill during cancellation.
      void TestCancelWithPreemptiveFill();

      //! Tests an Order that cancels on its own/pre-emptively.
      void TestPreemptiveCancel();

      //! Tests the continuation of a SingleOrderTask that was not filled.
      void TestUnfilledContinuation();

      //! Tests the continuation of a SingleOrderTask that was partially
      //! filled.
      void TestPartiallyFilledContinuation();

      //! Tests the continuation of a SingleOrderTask that was filled
      //! completely.
      void TestFilledContinuation();

    private:
      Beam::DelayPtr<OrderExecutionService::Tests::MockOrderExecutionClient>
        m_client;
      Beam::DelayPtr<OrderExecutionService::OrderExecutionMonitorSink>
        m_orderSink;
      Beam::DelayPtr<Beam::SignalHandling::SignalSink> m_taskSink;
      Beam::DelayPtr<Beam::SignalHandling::SignalSink> m_executionSink;

      CPPUNIT_TEST_SUITE(SingleOrderTaskTester);
        CPPUNIT_TEST(TestEmptyOrder);
        CPPUNIT_TEST(TestRejectedOrder);
        CPPUNIT_TEST(TestFullOrder);
        CPPUNIT_TEST(TestCancelPriorToPendingOrder);
        CPPUNIT_TEST(TestCancelWithPreemptiveFill);
        CPPUNIT_TEST(TestPreemptiveCancel);
        CPPUNIT_TEST(TestUnfilledContinuation);
        CPPUNIT_TEST(TestPartiallyFilledContinuation);
        CPPUNIT_TEST(TestFilledContinuation);
      CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
