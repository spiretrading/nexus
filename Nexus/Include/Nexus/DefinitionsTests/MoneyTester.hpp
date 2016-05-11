#ifndef NEXUS_MONEYTESTER_HPP
#define NEXUS_MONEYTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/DefinitionsTests/DefinitionsTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class MoneyTester
      \brief Tests the Money class.
   */
  class MoneyTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests converting a value to a string.
      void TestToString();

      //! Tests parsing a string.
      void TestFromString();

      //! Tests the floor function.
      void TestFloor();

      //! Tests the round function.
      void TestRound();

    private:
      CPPUNIT_TEST_SUITE(MoneyTester);
        CPPUNIT_TEST(TestToString);
        CPPUNIT_TEST(TestFromString);
        CPPUNIT_TEST(TestFloor);
        CPPUNIT_TEST(TestRound);
      CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
