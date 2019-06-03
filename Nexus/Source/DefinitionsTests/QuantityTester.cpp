#include "Nexus/DefinitionsTests/QuantityTester.hpp"
#include "Nexus/Definitions/Quantity.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace std;

void QuantityTester::TestToString() {
  CPPUNIT_ASSERT(ToString(Quantity(0)) == "0");
  CPPUNIT_ASSERT(ToString(Quantity(1)) == "1");
  CPPUNIT_ASSERT(ToString(Quantity(1.1)) == "1.100000");
}

void QuantityTester::TestFromString() {
  CPPUNIT_ASSERT(Quantity::FromValue("1") == Quantity(1));
  CPPUNIT_ASSERT(Quantity::FromValue("1.1") == Quantity(1.1));
}

void QuantityTester::TestAssignment() {
  auto q = Quantity(0);
  q = 100;
  CPPUNIT_ASSERT(q == 100);
}

void QuantityTester::TestModulus() {
  CPPUNIT_ASSERT(Quantity(1) % Quantity(5) == Quantity(1));
  CPPUNIT_ASSERT(Quantity(3) % Quantity(2) == Quantity(1));
}

void QuantityTester::TestFloor() {
  CPPUNIT_ASSERT(Floor(Quantity(1), 0) == Quantity(1));
  CPPUNIT_ASSERT(Floor(Quantity(-1), 0) == Quantity(-1));
  CPPUNIT_ASSERT(Floor(Quantity(10), -1) == Quantity(10));
  CPPUNIT_ASSERT(Floor(Quantity(-10), -1) == Quantity(-10));
  CPPUNIT_ASSERT(Floor(Quantity(10), 1) == Quantity(10));
  CPPUNIT_ASSERT(Floor(Quantity(-10), 1) == Quantity(-10));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("-1.5"), 0) == Quantity(-2));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("-1.5"), -1) == Quantity(-10));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("-0.5"), -1) == Quantity(-10));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("0.5"), -1) == Quantity(0));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("0.5"), 0) == Quantity(0));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("0.5"), 1) ==
    Quantity::FromValue("0.5"));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("0.5"), 2) ==
    Quantity::FromValue("0.5"));
  CPPUNIT_ASSERT(Floor(Quantity(5), 0) == Quantity(5));
  CPPUNIT_ASSERT(Floor(Quantity(5), -1) == Quantity(0));
  CPPUNIT_ASSERT(Floor(Quantity(5), -2) == Quantity(0));
  CPPUNIT_ASSERT(Floor(Quantity(37), 0) == Quantity(37));
  CPPUNIT_ASSERT(Floor(Quantity(37), -1) == Quantity(30));
  CPPUNIT_ASSERT(Floor(Quantity(37), -2) == Quantity(0));
  CPPUNIT_ASSERT(Floor(Quantity(33), 0) == Quantity(33));
  CPPUNIT_ASSERT(Floor(Quantity(33), -1) == Quantity(30));
  CPPUNIT_ASSERT(Floor(Quantity(33), -2) == Quantity(0));
  CPPUNIT_ASSERT(Floor(Quantity(73), 0) == Quantity(73));
  CPPUNIT_ASSERT(Floor(Quantity(73), -1) == Quantity(70));
  CPPUNIT_ASSERT(Floor(Quantity(73), -2) == Quantity(0));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("555.555"), -3) == Quantity(0));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("555.555"), -2) == Quantity(500));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("555.555"), -1) == Quantity(550));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("555.555"), 0) == Quantity(555));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("555.555"), 1) ==
    Quantity::FromValue("555.5"));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("555.555"), 2) ==
    Quantity::FromValue("555.55"));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("555.555"), 3) ==
    Quantity::FromValue("555.555"));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("555.555"), 4) ==
    Quantity::FromValue("555.555"));
  CPPUNIT_ASSERT(Floor(*Quantity::FromValue("-555.555"), 1) ==
    Quantity::FromValue("-555.6"));
}

void QuantityTester::TestCeil() {
  CPPUNIT_ASSERT(Ceil(Quantity(1), 0) == Quantity(1));
  CPPUNIT_ASSERT(Ceil(Quantity(-1), 0) == Quantity(-1));
  CPPUNIT_ASSERT(Ceil(Quantity(10), -1) == Quantity(10));
  CPPUNIT_ASSERT(Ceil(Quantity(-10), -1) == Quantity(-10));
  CPPUNIT_ASSERT(Ceil(Quantity(10), 1) == Quantity(10));
  CPPUNIT_ASSERT(Ceil(Quantity(-10), 1) == Quantity(-10));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("-1.5"), 0) == Quantity(-1));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("-1.5"), -1) == Quantity(0));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("-0.5"), -1) == Quantity(0));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("0.5"), -1) == Quantity(10));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("0.5"), 0) == Quantity(1));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("0.5"), 1) ==
    Quantity::FromValue("0.5"));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("0.5"), 2) ==
    Quantity::FromValue("0.5"));
  CPPUNIT_ASSERT(Ceil(Quantity(5), 0) == Quantity(5));
  CPPUNIT_ASSERT(Ceil(Quantity(5), -1) == Quantity(10));
  CPPUNIT_ASSERT(Ceil(Quantity(5), -2) == Quantity(100));
  CPPUNIT_ASSERT(Ceil(Quantity(37), 0) == Quantity(37));
  CPPUNIT_ASSERT(Ceil(Quantity(37), -1) == Quantity(40));
  CPPUNIT_ASSERT(Ceil(Quantity(37), -2) == Quantity(100));
  CPPUNIT_ASSERT(Ceil(Quantity(33), 0) == Quantity(33));
  CPPUNIT_ASSERT(Ceil(Quantity(33), -1) == Quantity(40));
  CPPUNIT_ASSERT(Ceil(Quantity(33), -2) == Quantity(100));
  CPPUNIT_ASSERT(Ceil(Quantity(73), 0) == Quantity(73));
  CPPUNIT_ASSERT(Ceil(Quantity(73), -1) == Quantity(80));
  CPPUNIT_ASSERT(Ceil(Quantity(73), -2) == Quantity(100));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("555.555"), -3) == Quantity(1000));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("555.555"), -2) == Quantity(600));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("555.555"), -1) == Quantity(560));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("555.555"), 0) == Quantity(556));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("555.555"), 1) ==
    Quantity::FromValue("555.6"));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("555.555"), 2) ==
    Quantity::FromValue("555.56"));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("555.555"), 3) ==
    Quantity::FromValue("555.555"));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("555.555"), 4) ==
    Quantity::FromValue("555.555"));
  CPPUNIT_ASSERT(Ceil(*Quantity::FromValue("-555.555"), 1) ==
    Quantity::FromValue("-555.5"));
}

void QuantityTester::TestRound() {
  CPPUNIT_ASSERT(Round(*Quantity::FromValue("0.5"), 0) == Quantity(1));
  CPPUNIT_ASSERT(Round(Quantity(5), -2) == Quantity(0));
  CPPUNIT_ASSERT(Round(Quantity(37), -2) == Quantity(0));
  CPPUNIT_ASSERT(Round(Quantity(33), -2) == Quantity(0));
  CPPUNIT_ASSERT(Round(Quantity(37), -3) == Quantity(0));
  CPPUNIT_ASSERT(Round(Quantity(73), -2) == Quantity(100));
  CPPUNIT_ASSERT(Round(Quantity(73), -2) == Quantity(100));
}
