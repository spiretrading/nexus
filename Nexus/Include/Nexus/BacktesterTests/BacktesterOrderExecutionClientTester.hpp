#ifndef NEXUS_BACKTESTERORDEREXECUTIONCLIENTTESTER_HPP
#define NEXUS_BACKTESTERORDEREXECUTIONCLIENTTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/BacktesterTests/BacktesterTests.hpp"

namespace Nexus {
namespace Tests {

  /*! \class BacktesterOrderExecutionClientTester
      \brief Tests the BacktesterOrderExecutionClient class.
   */
  class BacktesterOrderExecutionClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests submitting an Order and having it get filled.
      void TestOrderSubmissionAndFill();

      //! Tests submitting an Order and cancelling it.
      void TestOrderSubmissionAndCancel();

    private:
      CPPUNIT_TEST_SUITE(BacktesterOrderExecutionClientTester);
        CPPUNIT_TEST(TestOrderSubmissionAndFill);
        CPPUNIT_TEST(TestOrderSubmissionAndCancel);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
