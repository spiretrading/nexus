#ifndef NEXUS_BOOKKEEPER_REACTOR_TESTER_HPP
#define NEXUS_BOOKKEEPER_REACTOR_TESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/AccountingTests/AccountingTests.hpp"

namespace Nexus::Accounting::Tests {

  /* Tests the BookkeeperReactor. */
  class BookkeeperReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a single order's execution reports.
      void TestSingleOrder();

    private:
      CPPUNIT_TEST_SUITE(BookkeeperReactorTester);
        CPPUNIT_TEST(TestSingleOrder);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}

#endif
