#ifndef NEXUS_RISKSTATETRACKERTESTER_HPP
#define NEXUS_RISKSTATETRACKERTESTER_HPP
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/RiskService/RiskStateTracker.hpp"
#include "Nexus/RiskServiceTests/RiskServiceTests.hpp"

namespace Nexus {
namespace RiskService {
namespace Tests {

  /*! \class RiskStateTrackerTester
      \brief Tests the RiskStateTracker class.
   */
  class RiskStateTrackerTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of Portfolio used.
      typedef Accounting::Portfolio<Accounting::TrueAverageBookkeeper<
        Accounting::Inventory<Accounting::Position<Security>>>> Portfolio;

      //! The type of RiskStateTracker to test.
      typedef RiskService::RiskStateTracker<Portfolio,
        Beam::TimeService::IncrementalTimeClient> RiskStateTracker;

      //! Tests opening a position.
      void TestOpenPosition();

    private:
      CPPUNIT_TEST_SUITE(RiskStateTrackerTester);
        CPPUNIT_TEST(TestOpenPosition);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
