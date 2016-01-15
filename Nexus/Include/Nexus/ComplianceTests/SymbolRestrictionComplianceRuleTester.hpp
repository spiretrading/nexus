#ifndef NEXUS_SYMBOLRESTRICTIONCOMPLIANCERULETESTER_HPP
#define NEXUS_SYMBOLRESTRICTIONCOMPLIANCERULETESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class SymbolRestrictionComplianceRuleTester
      \brief Tests the SymbolRestrictionComplianceRule class.
   */
  class SymbolRestrictionComplianceRuleTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Construct the rule.
      //! Submit an order on TST1.TSX, the order should be valid.
      void TestEmptyRestriction();

      //! Construct the rule and restrict TST1.TSX.
      //! Submit an order on TST1.TSX, the order should be rejected.
      //! Submit an order on TST2.TSX, the order should be valid.
      //! Submit an order on TST1.ASX, the order should be valid.
      void TestSingleSymbolSubmissionAndRestriction();

    private:
      CPPUNIT_TEST_SUITE(SymbolRestrictionComplianceRuleTester);
        CPPUNIT_TEST(TestEmptyRestriction);
        CPPUNIT_TEST(TestSingleSymbolSubmissionAndRestriction);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
