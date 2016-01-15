#ifndef NEXUS_SHORTINGTRACKERTESTER_HPP
#define NEXUS_SHORTINGTRACKERTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/AccountingTests/AccountingTests.hpp"

namespace Nexus {
namespace Accounting {
namespace Tests {

  /*  \class ShortingTrackerTester
      \brief Tests the ShortingTracker class.
   */
  class ShortingTrackerTester : public CPPUNIT_NS::TestFixture {
    public:

    private:
      CPPUNIT_TEST_SUITE(ShortingTrackerTester);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
