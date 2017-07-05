#include "Nexus/DefinitionsTests/MoneyTester.hpp"
#include "Nexus/Definitions/Money.hpp"

using namespace Nexus;
using namespace Nexus::Tests;
using namespace std;

void MoneyTester::TestToString() {
  CPPUNIT_ASSERT(Money::ZERO.ToString() == "0.00");
  CPPUNIT_ASSERT(Money::ONE.ToString() == "1.00");
  CPPUNIT_ASSERT((Money::ONE / 10).ToString() == "0.10");
  CPPUNIT_ASSERT((Money::ONE / 100).ToString() == "0.01");
  CPPUNIT_ASSERT((Money::ONE / 1000).ToString() == "0.001");
  CPPUNIT_ASSERT((Money::ONE / 10000).ToString() == "0.0001");
  CPPUNIT_ASSERT((Money::ONE / 100000).ToString() == "0.00001");
  CPPUNIT_ASSERT((Money::ONE / 1000000).ToString() == "0.000001");
  CPPUNIT_ASSERT((13 * Money::CENT + Money::CENT / 2).ToString() == "0.135");
  CPPUNIT_ASSERT((3 * Money::CENT + Money::CENT / 2).ToString() == "0.035");
}

void MoneyTester::TestFromString() {
  CPPUNIT_ASSERT(Money::FromValue("0") == Money::ZERO);
  CPPUNIT_ASSERT(Money::FromValue("-0") == Money::ZERO);
  CPPUNIT_ASSERT(Money::FromValue("+0") == Money::ZERO);
  CPPUNIT_ASSERT(Money::FromValue("+.") == Money::ZERO);
  CPPUNIT_ASSERT(Money::FromValue("-.") == Money::ZERO);
  CPPUNIT_ASSERT(Money::FromValue("0.01") == Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("0.02") == 2 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("0.10") == 10 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("0.11") == 11 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("0.12") == 12 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("1.00") == Money::ONE);
  CPPUNIT_ASSERT(Money::FromValue("1.01") == Money::ONE + Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("1.02") == Money::ONE + 2 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("1.10") == Money::ONE + 10 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("1.11") == Money::ONE + 11 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("1.12") == Money::ONE + 12 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("2.01") == 2 * Money::ONE + Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+0.01") == Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+0.02") == 2 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+0.10") == 10 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+0.11") == 11 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+0.12") == 12 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+1.00") == Money::ONE);
  CPPUNIT_ASSERT(Money::FromValue("+1.01") == Money::ONE + Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+1.02") == Money::ONE + 2 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+1.10") == Money::ONE + 10 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+1.11") == Money::ONE + 11 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+1.12") == Money::ONE + 12 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+2.01") == 2 * Money::ONE + Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("-0.01") == -Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("-0.02") == -(2 * Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("-0.10") == -(10 * Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("-0.11") == -(11 * Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("-0.12") == -(12 * Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("-1.00") == -Money::ONE);
  CPPUNIT_ASSERT(Money::FromValue("-1.01") == -(Money::ONE + Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("-1.02") == -(Money::ONE + 2 * Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("-1.10") == -(Money::ONE + 10 * Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("-1.11") == -(Money::ONE + 11 * Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("-1.12") == -(Money::ONE + 12 * Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("-2.01") == -(2 * Money::ONE + Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue(".01") == Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue(".02") == 2 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue(".10") == 10 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue(".11") == 11 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue(".12") == 12 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+.01") == Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+.02") == 2 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+.10") == 10 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+.11") == 11 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("+.12") == 12 * Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("-.01") == -Money::CENT);
  CPPUNIT_ASSERT(Money::FromValue("-.02") == -(2 * Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("-.10") == -(10 * Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("-.11") == -(11 * Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("-.12") == -(12 * Money::CENT));
  CPPUNIT_ASSERT(Money::FromValue("0.00000100") == (Money::ONE / 1000000));
}

void MoneyTester::TestFloor() {
  Money value = 2.055 * Money::CENT;
  Money flooredValue = Floor(value, 2);
  CPPUNIT_ASSERT(flooredValue == 2 * Money::CENT);
}

void MoneyTester::TestRound() {
  {
    auto value = Money::FromValue("75.504");
    auto roundValue = Round(*value, 2);
    CPPUNIT_ASSERT(roundValue == Money::FromValue("75.50"));
  }
  {
    auto value = Money::FromValue("75.505");
    auto roundValue = Round(*value, 2);
    CPPUNIT_ASSERT(roundValue == Money::FromValue("75.51"));
  }
  {
    auto value = Money::FromValue("75.506");
    auto roundValue = Round(*value, 2);
    CPPUNIT_ASSERT(roundValue == Money::FromValue("75.51"));
  }
}
