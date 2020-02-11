#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Nexus/AccountingTests/BookkeeperReactorTester.hpp"
#include "Nexus/AccountingTests/BuyingPowerTrackerTester.hpp"
#include "Nexus/AccountingTests/PortfolioMonitorTester.hpp"
#include "Nexus/AccountingTests/PositionOrderBookTester.hpp"
#include "Nexus/AccountingTests/ShortingTrackerTester.hpp"
#include "Nexus/AccountingTests/TrueAverageBookkeeperTester.hpp"

using namespace Nexus::Accounting::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(BuyingPowerTrackerTester::suite());
  runner.addTest(PortfolioMonitorTester::suite());
  runner.addTest(ShortingTrackerTester::suite());
  runner.addTest(TrueAverageBookkeeperTester::suite());
  runner.addTest(PositionOrderBookTester::suite());
  runner.addTest(BookkeeperReactorTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  bool wasSucessful = runner.run();
  return wasSucessful ? 0 : -1;
}
