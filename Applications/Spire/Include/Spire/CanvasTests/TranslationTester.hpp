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

    private:
      CPPUNIT_TEST_SUITE(TranslationTester);
        CPPUNIT_TEST(TestTranslatingConstant);
      CPPUNIT_TEST_SUITE_END();
  };
}

#endif
