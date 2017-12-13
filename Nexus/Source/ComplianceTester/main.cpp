#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Nexus/ComplianceTests/BuyingPowerComplianceRuleTester.hpp"
#include "Nexus/ComplianceTests/CancelRestrictionPeriodComplianceRuleTester.hpp"
#include "Nexus/ComplianceTests/ComplianceRuleSetTester.hpp"
#include "Nexus/ComplianceTests/ComplianceServletTester.hpp"
#include "Nexus/ComplianceTests/ComplianceTests.hpp"
#include "Nexus/ComplianceTests/LocalComplianceRuleDataStoreTester.hpp"
#include "Nexus/ComplianceTests/OpposingOrderCancellationComplianceRuleTester.hpp"
#include "Nexus/ComplianceTests/OpposingOrderSubmissionComplianceRuleTester.hpp"
#include "Nexus/ComplianceTests/RejectCancelsComplianceRuleTester.hpp"
#include "Nexus/ComplianceTests/RejectSubmissionsComplianceRuleTester.hpp"
#include "Nexus/ComplianceTests/SecurityFilterComplianceRuleTester.hpp"
#include "Nexus/ComplianceTests/SubmissionRestrictionPeriodComplianceRuleTester.hpp"
#include "Nexus/ComplianceTests/SymbolRestrictionComplianceRuleTester.hpp"
#include "Nexus/ComplianceTests/TimeFilterComplianceRuleTester.hpp"

using namespace Nexus::Compliance::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(LocalComplianceRuleDataStoreTester::suite());
  runner.addTest(ComplianceRuleSetTester::suite());
  runner.addTest(BuyingPowerComplianceRuleTester::suite());
  runner.addTest(CancelRestrictionPeriodComplianceRuleTester::suite());
  runner.addTest(OpposingOrderCancellationComplianceRuleTester::suite());
  runner.addTest(OpposingOrderSubmissionComplianceRuleTester::suite());
  runner.addTest(RejectCancelsComplianceRuleTester::suite());
  runner.addTest(RejectSubmissionsComplianceRuleTester::suite());
  runner.addTest(SecurityFilterComplianceRuleTester::suite());
  runner.addTest(SubmissionRestrictionPeriodComplianceRuleTester::suite());
  runner.addTest(SymbolRestrictionComplianceRuleTester::suite());
  runner.addTest(TimeFilterComplianceRuleTester::suite());
  runner.addTest(ComplianceServletTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  bool wasSucessful = runner.run();
  return wasSucessful ? 0 : -1;
}
