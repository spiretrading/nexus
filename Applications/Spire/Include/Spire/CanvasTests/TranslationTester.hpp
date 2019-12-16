#ifndef SPIRE_TRANSLATION_TESTER_HPP
#define SPIRE_TRANSLATION_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Spire/CanvasTests/CanvasTests.hpp"

namespace Spire::Tests {

  /** Tests canvas translation operations. */
  class TranslationTester : public CPPUNIT_NS::TestFixture {
    public:

      /** Tests translating a constant. */
      void TestTranslatingConstant();

      /** Tests translating a chain. */
      void TestTranslatingChain();

      /**
       * Tests translating a chain containing a reference from the last child
       * to the first child.
       */
      void TestTranslatingChainWithTailReference();

      /**
       * Tests translating a chain containing a reference from the first child
       * to the last child.
       */
      void TestTranslatingChainWithHeadReference();

      /** Tests translation an order. */
      void TestTranslatingOrder();

      /** Tests translation an order as a Task. */
      void TestTranslatingOrderTask();

    private:
      CPPUNIT_TEST_SUITE(TranslationTester);
        CPPUNIT_TEST(TestTranslatingConstant);
        CPPUNIT_TEST(TestTranslatingChain);
        CPPUNIT_TEST(TestTranslatingChainWithTailReference);
        CPPUNIT_TEST(TestTranslatingChainWithHeadReference);
        CPPUNIT_TEST(TestTranslatingOrder);
        CPPUNIT_TEST(TestTranslatingOrderTask);
      CPPUNIT_TEST_SUITE_END();
  };
}

#endif
