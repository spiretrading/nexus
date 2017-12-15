#ifndef NEXUS_QUANTITY_TESTER_HPP
#define NEXUS_QUANTITY_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/DefinitionsTests/DefinitionsTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class QuantityTester
      \brief Tests the Quantity class.
   */
  class QuantityTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests converting a value to a string.
      void TestToString();

      //! Tests parsing a string.
      void TestFromString();

      //! Tests assigning to a Quantity.
      void TestAssignment();

    private:
      CPPUNIT_TEST_SUITE(QuantityTester);
        CPPUNIT_TEST(TestToString);
        CPPUNIT_TEST(TestFromString);
        CPPUNIT_TEST(TestAssignment);
      CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
