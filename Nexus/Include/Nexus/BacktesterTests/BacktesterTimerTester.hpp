#ifndef NEXUS_BACKTESTERTIMERTESTER_HPP
#define NEXUS_BACKTESTERTIMERTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/BacktesterTests/BacktesterTests.hpp"

namespace Nexus {
namespace Tests {

  /*! \class BacktesterTimerTester
      \brief Tests the BacktesterTimer class.
   */
  class BacktesterTimerTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests allowing a timer to expire.
      void TestExpiry();

      //! Tests canceling a timer.
      void TestCancel();

    private:
      CPPUNIT_TEST_SUITE(BacktesterTimerTester);
        CPPUNIT_TEST(TestExpiry);
        CPPUNIT_TEST(TestCancel);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
