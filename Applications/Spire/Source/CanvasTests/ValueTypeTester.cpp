#include <doctest/doctest.h>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Canvas/Types/BooleanType.hpp"
#include "Spire/Canvas/Types/CurrencyType.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/Types/DecimalType.hpp"
#include "Spire/Canvas/Types/DestinationType.hpp"
#include "Spire/Canvas/Types/DurationType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
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
#include "Spire/Canvas/Types/VenueType.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

TEST_SUITE("ValueType") {
  TEST_CASE("names") {
    REQUIRE(BooleanType::GetInstance().GetName() == "Boolean");
    REQUIRE(CurrencyType::GetInstance().GetName() == "Currency");
    REQUIRE(DateTimeType::GetInstance().GetName() == "Date/Time");
    REQUIRE(DecimalType::GetInstance().GetName() == "Decimal");
    REQUIRE(DestinationType::GetInstance().GetName() == "Destination");
    REQUIRE(DurationType::GetInstance().GetName() == "Duration");
    REQUIRE(IntegerType::GetInstance().GetName() == "Integer");
    REQUIRE(MoneyType::GetInstance().GetName() == "Money");
    REQUIRE(OrderReferenceType::GetInstance().GetName() == "Order");
    REQUIRE(OrderStatusType::GetInstance().GetName() == "Order Status");
    REQUIRE(OrderTypeType::GetInstance().GetName() == "Order Type");
    REQUIRE(SecurityType::GetInstance().GetName() == "Security");
    REQUIRE(SequenceType::GetInstance().GetName() == "Sequence");
    REQUIRE(SideType::GetInstance().GetName() == "Side");
    REQUIRE(TextType::GetInstance().GetName() == "Text");
    REQUIRE(TimeInForceType::GetInstance().GetName() == "Time In Force");
    REQUIRE(TimeRangeType::GetInstance().GetName() == "Time Range");
    REQUIRE(VenueType::GetInstance().GetName() == "Venue");
  }

  TEST_CASE("native_types") {
    REQUIRE(BooleanType::GetInstance().GetNativeType() == typeid(bool));
    REQUIRE(CurrencyType::GetInstance().GetNativeType() ==
      typeid(CurrencyId));
    REQUIRE(DateTimeType::GetInstance().GetNativeType() == typeid(ptime));
    REQUIRE(DecimalType::GetInstance().GetNativeType() == typeid(double));
    REQUIRE(DestinationType::GetInstance().GetNativeType() ==
      typeid(std::string));
    REQUIRE(DurationType::GetInstance().GetNativeType() ==
      typeid(time_duration));
    REQUIRE(IntegerType::GetInstance().GetNativeType() ==
      typeid(Quantity));
    REQUIRE(MoneyType::GetInstance().GetNativeType() == typeid(Money));
    REQUIRE(OrderReferenceType::GetInstance().GetNativeType() ==
      typeid(std::shared_ptr<Order>));
    REQUIRE(OrderStatusType::GetInstance().GetNativeType() ==
      typeid(OrderStatus));
    REQUIRE(OrderTypeType::GetInstance().GetNativeType() ==
      typeid(OrderType));
    REQUIRE(SecurityType::GetInstance().GetNativeType() ==
      typeid(Security));
    REQUIRE(SequenceType::GetInstance().GetNativeType() ==
      typeid(Beam::Sequence));
    REQUIRE(SideType::GetInstance().GetNativeType() == typeid(Side));
    REQUIRE(TextType::GetInstance().GetNativeType() == typeid(std::string));
    REQUIRE(TimeInForceType::GetInstance().GetNativeType() ==
      typeid(TimeInForce));
    REQUIRE(TimeRangeType::GetInstance().GetNativeType() ==
      typeid(Beam::Range));
    REQUIRE(VenueType::GetInstance().GetNativeType() == typeid(Venue));
  }

  TEST_CASE("compatibility") {
    REQUIRE(BooleanType::GetInstance().GetCompatibility(
      BooleanType::GetInstance()) == CanvasType::Compatibility::EQUAL);
    REQUIRE(BooleanType::GetInstance().GetCompatibility(
      IntegerType::GetInstance()) == CanvasType::Compatibility::NONE);
    REQUIRE(BooleanType::GetInstance().GetCompatibility(
      IntegerType::GetInstance()) == CanvasType::Compatibility::NONE);
  }
}
