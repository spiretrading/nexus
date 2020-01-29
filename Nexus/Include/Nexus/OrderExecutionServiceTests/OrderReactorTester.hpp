#ifndef NEXUS_ORDER_REACTOR_TESTER_HPP
#define NEXUS_ORDER_REACTOR_TESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>

namespace Nexus::OrderExecutionService::Tests {

  /* Tests the OrderReactor class. */
  class OrderReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      /** Tests an OrderFields reactor that immediately completes producing
       *  no values.
       */
      void TestEmptyOrderFields();

      /** Tests an OrderFields reactor that produces only a single value. */
      void TestSingleOrderFields();

      /** Tests updating the OrderFields. */
      void TestOrderFieldsUpdate();

      /** Tests a partial fill followed by an update to the OrderFields. */
      void TestPartialFillAndUpdate();

      /** Tests a terminal order along with an update to the OrderFields on the
       *  same commit.
       */
      void TestTerminalOrderAndUpdate();

      /** Tests delaying the quantity field. */
      void TestDelayedQuantity();

    private:
      CPPUNIT_TEST_SUITE(OrderReactorTester);
        CPPUNIT_TEST(TestEmptyOrderFields);
        CPPUNIT_TEST(TestSingleOrderFields);
        CPPUNIT_TEST(TestOrderFieldsUpdate);
        CPPUNIT_TEST(TestPartialFillAndUpdate);
        CPPUNIT_TEST(TestTerminalOrderAndUpdate);
        CPPUNIT_TEST(TestDelayedQuantity);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}

#endif
