#ifndef SPIRE_VALUETYPETESTER_HPP
#define SPIRE_VALUETYPETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Spire/CanvasTests/CanvasTests.hpp"

namespace Spire {
namespace Tests {

  /*  \class ValueTypeTester
      \brief Tests the ValueType class.
   */
  class ValueTypeTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests the names of all ValueTypes.
      void TestNames();

      //! Tests the type ids of all ValueTypes.
      void TestNativeTypes();

      //! Tests type compatibility among ValueTypes.
      void TestCompatibility();

    private:
      CPPUNIT_TEST_SUITE(ValueTypeTester);
        CPPUNIT_TEST(TestNames);
        CPPUNIT_TEST(TestNativeTypes);
        CPPUNIT_TEST(TestCompatibility);
      CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
