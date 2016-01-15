#include "Nexus/DefinitionsTests/MoneyTester.hpp"
#include "Nexus/Definitions/Money.hpp"

using namespace Nexus;
using namespace Nexus::Tests;
using namespace std;

void MoneyTester::TestToString() {
  CPPUNIT_ASSERT(Money::FromRepresentation(0).ToString() == "0.00");
  CPPUNIT_ASSERT(Money::FromRepresentation(1000000).ToString() == "1.00");
  CPPUNIT_ASSERT(Money::FromRepresentation(100000).ToString() == "0.10");
  CPPUNIT_ASSERT(Money::FromRepresentation(10000).ToString() == "0.01");
  CPPUNIT_ASSERT(Money::FromRepresentation(1000).ToString() == "0.001");
  CPPUNIT_ASSERT(Money::FromRepresentation(100).ToString() == "0.0001");
  CPPUNIT_ASSERT(Money::FromRepresentation(10).ToString() == "0.00001");
  CPPUNIT_ASSERT(Money::FromRepresentation(1).ToString() == "0.000001");
  CPPUNIT_ASSERT(Money::FromRepresentation(135000).ToString() == "0.135");
  CPPUNIT_ASSERT(Money::FromRepresentation(35000).ToString() == "0.035");
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
}

void MoneyTester::TestFloor() {
  Money value = 2.055 * Money::CENT;
  Money flooredValue = Floor(value, 2);
  CPPUNIT_ASSERT(flooredValue == 2 * Money::CENT);
}
