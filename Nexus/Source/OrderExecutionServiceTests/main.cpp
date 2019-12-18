#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Nexus/OrderExecutionServiceTests/BoardLotCheckTester.hpp"
#include "Nexus/OrderExecutionServiceTests/BuyingPowerCheckTester.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionClientTester.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServletTester.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderReactorTester.hpp"

using namespace Nexus::OrderExecutionService::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(BoardLotCheckTester::suite());
  runner.addTest(BuyingPowerCheckTester::suite());
  runner.addTest(OrderExecutionClientTester::suite());
  runner.addTest(OrderExecutionServletTester::suite());
  runner.addTest(OrderReactorTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  bool wasSucessful = runner.run();
  return wasSucessful ? 0 : -1;
}
