#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Nexus/FeeHandlingTests/AsxtFeeHandlingTester.hpp"
#include "Nexus/FeeHandlingTests/ChicFeeHandlingTester.hpp"
#include "Nexus/FeeHandlingTests/ConsolidatedTmxFeeTableTester.hpp"
#include "Nexus/FeeHandlingTests/ConsolidatedUsFeeTableTester.hpp"
#include "Nexus/FeeHandlingTests/LynxFeeHandlingTester.hpp"
#include "Nexus/FeeHandlingTests/MatnFeeHandlingTester.hpp"
#include "Nexus/FeeHandlingTests/NeoeFeeHandlingTester.hpp"
#include "Nexus/FeeHandlingTests/NsdqFeeHandlingTester.hpp"
#include "Nexus/FeeHandlingTests/OmgaFeeHandlingTester.hpp"
#include "Nexus/FeeHandlingTests/PureFeeHandlingTester.hpp"
#include "Nexus/FeeHandlingTests/TsxFeeHandlingTester.hpp"
#include "Nexus/FeeHandlingTests/XatsFeeHandlingTester.hpp"
#include "Nexus/FeeHandlingTests/Xcx2FeeHandlingTester.hpp"

using namespace Nexus::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(AsxtFeeHandlingTester::suite());
  runner.addTest(ChicFeeHandlingTester::suite());
  runner.addTest(LynxFeeHandlingTester::suite());
  runner.addTest(MatnFeeHandlingTester::suite());
  runner.addTest(NeoeFeeHandlingTester::suite());
  runner.addTest(OmgaFeeHandlingTester::suite());
  runner.addTest(PureFeeHandlingTester::suite());
  runner.addTest(TsxFeeHandlingTester::suite());
  runner.addTest(XatsFeeHandlingTester::suite());
  runner.addTest(Xcx2FeeHandlingTester::suite());
  runner.addTest(ConsolidatedTmxFeeTableTester::suite());
  runner.addTest(NsdqFeeHandlingTester::suite());
  runner.addTest(ConsolidatedUsFeeTableTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  bool wasSucessful = runner.run();
  return wasSucessful ? 0 : -1;
}
