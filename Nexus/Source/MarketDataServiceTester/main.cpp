#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Nexus/MarketDataServiceTests/LocalHistoricalDataStoreTester.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataClientTester.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataFeedClientTester.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataRegistryTester.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataRegistryServletTester.hpp"
#include "Nexus/MarketDataServiceTests/MarketEntryTester.hpp"
#include "Nexus/MarketDataServiceTests/SecurityEntryTester.hpp"

using namespace Nexus::MarketDataService::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(MarketDataClientTester::suite());
  runner.addTest(MarketDataFeedClientTester::suite());
  runner.addTest(MarketEntryTester::suite());
  runner.addTest(SecurityEntryTester::suite());
  runner.addTest(MarketDataRegistryTester::suite());
  runner.addTest(LocalHistoricalDataStoreTester::suite());
  runner.addTest(MarketDataRegistryServletTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  bool wasSucessful = runner.run();
  return wasSucessful ? 0 : -1;
}
