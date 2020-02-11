#include "Spire/CanvasTests/ValueTypeTester.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Canvas/Types/BooleanType.hpp"
#include "Spire/Canvas/Types/CurrencyType.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/Types/DecimalType.hpp"
#include "Spire/Canvas/Types/DestinationType.hpp"
#include "Spire/Canvas/Types/DurationType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MarketType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/Types/OrderStatusType.hpp"
#include "Spire/Canvas/Types/OrderTypeType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/SecurityType.hpp"
#include "Spire/Canvas/Types/SequenceType.hpp"
#include "Spire/Canvas/Types/SideType.hpp"
#include "Spire/Canvas/Types/TextType.hpp"
#include "Spire/Canvas/Types/TimeInForceType.hpp"
#include "Spire/Canvas/Types/TimeRangeType.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::Tests;
using namespace std;

void ValueTypeTester::TestNames() {
  CPPUNIT_ASSERT(BooleanType::GetInstance().GetName() == "Boolean");
  CPPUNIT_ASSERT(CurrencyType::GetInstance().GetName() == "Currency");
  CPPUNIT_ASSERT(DateTimeType::GetInstance().GetName() == "Date/Time");
  CPPUNIT_ASSERT(DecimalType::GetInstance().GetName() == "Decimal");
  CPPUNIT_ASSERT(DestinationType::GetInstance().GetName() == "Destination");
  CPPUNIT_ASSERT(DurationType::GetInstance().GetName() == "Duration");
  CPPUNIT_ASSERT(IntegerType::GetInstance().GetName() == "Integer");
  CPPUNIT_ASSERT(MarketType::GetInstance().GetName() == "Market");
  CPPUNIT_ASSERT(MoneyType::GetInstance().GetName() == "Money");
  CPPUNIT_ASSERT(OrderReferenceType::GetInstance().GetName() == "Order");
  CPPUNIT_ASSERT(OrderStatusType::GetInstance().GetName() == "Order Status");
  CPPUNIT_ASSERT(OrderTypeType::GetInstance().GetName() == "Order Type");
  CPPUNIT_ASSERT(SecurityType::GetInstance().GetName() == "Security");
  CPPUNIT_ASSERT(SequenceType::GetInstance().GetName() == "Sequence");
  CPPUNIT_ASSERT(SideType::GetInstance().GetName() == "Side");
  CPPUNIT_ASSERT(TextType::GetInstance().GetName() == "Text");
  CPPUNIT_ASSERT(TimeInForceType::GetInstance().GetName() == "Time In Force");
  CPPUNIT_ASSERT(TimeRangeType::GetInstance().GetName() == "Time Range");
}

void ValueTypeTester::TestNativeTypes() {
  CPPUNIT_ASSERT(BooleanType::GetInstance().GetNativeType() == typeid(bool));
  CPPUNIT_ASSERT(CurrencyType::GetInstance().GetNativeType() ==
    typeid(CurrencyId));
  CPPUNIT_ASSERT(DateTimeType::GetInstance().GetNativeType() == typeid(ptime));
  CPPUNIT_ASSERT(DecimalType::GetInstance().GetNativeType() == typeid(double));
  CPPUNIT_ASSERT(DestinationType::GetInstance().GetNativeType() ==
    typeid(string));
  CPPUNIT_ASSERT(DurationType::GetInstance().GetNativeType() ==
    typeid(time_duration));
  CPPUNIT_ASSERT(IntegerType::GetInstance().GetNativeType() ==
    typeid(Quantity));
  CPPUNIT_ASSERT(MarketType::GetInstance().GetNativeType() ==
    typeid(MarketCode));
  CPPUNIT_ASSERT(MoneyType::GetInstance().GetNativeType() == typeid(Money));
  CPPUNIT_ASSERT(OrderReferenceType::GetInstance().GetNativeType() ==
    typeid(const Order*));
  CPPUNIT_ASSERT(OrderStatusType::GetInstance().GetNativeType() ==
    typeid(OrderStatus));
  CPPUNIT_ASSERT(OrderTypeType::GetInstance().GetNativeType() ==
    typeid(OrderType));
  CPPUNIT_ASSERT(SecurityType::GetInstance().GetNativeType() ==
    typeid(Security));
  CPPUNIT_ASSERT(SequenceType::GetInstance().GetNativeType() ==
    typeid(Beam::Queries::Sequence));
  CPPUNIT_ASSERT(SideType::GetInstance().GetNativeType() == typeid(Side));
  CPPUNIT_ASSERT(TextType::GetInstance().GetNativeType() == typeid(string));
  CPPUNIT_ASSERT(TimeInForceType::GetInstance().GetNativeType() ==
    typeid(TimeInForce));
  CPPUNIT_ASSERT(TimeRangeType::GetInstance().GetNativeType() ==
    typeid(Queries::Range));
}

void ValueTypeTester::TestCompatibility() {
  CPPUNIT_ASSERT(BooleanType::GetInstance().GetCompatibility(
    BooleanType::GetInstance()) == CanvasType::Compatibility::EQUAL);
  CPPUNIT_ASSERT(BooleanType::GetInstance().GetCompatibility(
    IntegerType::GetInstance()) == CanvasType::Compatibility::NONE);
  CPPUNIT_ASSERT(BooleanType::GetInstance().GetCompatibility(
    IntegerType::GetInstance()) == CanvasType::Compatibility::NONE);
}
