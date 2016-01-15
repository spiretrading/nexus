#ifndef NEXUS_TIMEZONETESTER_HPP
#define NEXUS_TIMEZONETESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/DefinitionsTests/DefinitionsTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class TimeZoneTester
      \brief Tests various time zone functions.
   */
  class TimeZoneTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Test retrieving the start of day on TSX.
      void TestTsxStartOfDay();

    private:
      CPPUNIT_TEST_SUITE(TimeZoneTester);
        CPPUNIT_TEST(TestTsxStartOfDay);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
