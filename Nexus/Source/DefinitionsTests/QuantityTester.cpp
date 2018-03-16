#include "Nexus/DefinitionsTests/QuantityTester.hpp"
#include "Nexus/Definitions/Quantity.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace std;

void QuantityTester::TestToString() {
  CPPUNIT_ASSERT(ToString(Quantity{0}) == "0");
  CPPUNIT_ASSERT(ToString(Quantity{1}) == "1");
  CPPUNIT_ASSERT(ToString(Quantity{1.1}) == "1.100000");
}

void QuantityTester::TestFromString() {
  CPPUNIT_ASSERT(Quantity::FromValue("1") == Quantity{1});
  CPPUNIT_ASSERT(Quantity::FromValue("1.1") == Quantity{1.1});
}

void QuantityTester::TestAssignment() {
  Quantity q{0};
  q = 100;
  CPPUNIT_ASSERT(q == 100);
}

void QuantityTester::TestModulus() {
  CPPUNIT_ASSERT(Quantity{1} % Quantity{5} == Quantity{1});
}
