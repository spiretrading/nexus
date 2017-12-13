#ifndef NEXUS_CONSOLIDATEDUSFEETABLETESTER_HPP
#define NEXUS_CONSOLIDATEDUSFEETABLETESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class ConsolidatedUsFeeTableTester
      \brief Tests fee handling by the ConsolidatedTmxFeeTable.
   */
  class ConsolidatedUsFeeTableTester : public CPPUNIT_NS::TestFixture {
    public:

    private:
      CPPUNIT_TEST_SUITE(ConsolidatedUsFeeTableTester);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
