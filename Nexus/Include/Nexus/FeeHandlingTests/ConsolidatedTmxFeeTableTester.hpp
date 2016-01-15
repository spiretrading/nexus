#ifndef NEXUS_CONSOLIDATEDTMXFEETABLETESTER_HPP
#define NEXUS_CONSOLIDATEDTMXFEETABLETESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class ConsolidatedTmxFeeTableTester
      \brief Tests fee handling by the ConsolidatedTmxFeeTable.
   */
  class ConsolidatedTmxFeeTableTester : public CPPUNIT_NS::TestFixture {
    public:

    private:
      CPPUNIT_TEST_SUITE(ConsolidatedTmxFeeTableTester);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
