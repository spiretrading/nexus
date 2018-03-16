#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Nexus/DefinitionsTests/MoneyTester.hpp"
#include "Nexus/DefinitionsTests/QuantityTester.hpp"
#include "Nexus/DefinitionsTests/RegionTester.hpp"
#include "Nexus/DefinitionsTests/RegionMapTester.hpp"
#include "Nexus/DefinitionsTests/SecuritySetTester.hpp"
#include "Nexus/DefinitionsTests/TimeZoneTester.hpp"

using namespace Nexus::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(MoneyTester::suite());
  runner.addTest(QuantityTester::suite());
  runner.addTest(RegionTester::suite());
  runner.addTest(RegionMapTester::suite());
  runner.addTest(TimeZoneTester::suite());
  runner.addTest(SecuritySetTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  bool wasSucessful = runner.run();
  return wasSucessful ? 0 : -1;
}
