#include <doctest/doctest.h>
#include "Nexus/FixUtilities/FixConversions.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("FixConversions") {
  TEST_CASE("timestamp_round_trip") {
    auto time = time_from_string("2023-08-17 13:45:12.123");
    auto fix_time = get_timestamp(time);
    auto round_trip = get_timestamp(fix_time);
    REQUIRE(time == round_trip);
  }

  TEST_CASE("utc_timestamp_round_trip") {
    auto time = time_from_string("2023-08-17 13:45:12.456");
    auto fix_utc = get_utc_timestamp(time);
    auto round_trip = get_utc_timestamp(fix_utc);
    REQUIRE(time == round_trip);
  }

  TEST_CASE("utc_timestamp_from_fix") {
    auto fix_utc = FIX::UtcTimeStamp(13, 45, 12, 789, 17, 8, 2023);
    auto time = get_utc_timestamp(fix_utc);
    REQUIRE(time == time_from_string("2023-08-17 13:45:12.789"));
  }

  TEST_CASE("timestamp_from_fix") {
    auto fix_time = FIX::DateTime(2023, 8, 17, 13, 45, 12, 321);
    auto time = get_timestamp(fix_time);
    REQUIRE(time == time_from_string("2023-08-17 13:45:12.321"));
  }

  TEST_CASE("order_status_fix_to_enum") {
    REQUIRE(*get_order_status(FIX::OrdStatus(FIX::OrdStatus_NEW)) ==
      Nexus::OrderStatus::NEW);
    REQUIRE(*get_order_status(FIX::OrdStatus(FIX::OrdStatus_FILLED)) ==
      Nexus::OrderStatus::FILLED);
    REQUIRE(!get_order_status(FIX::OrdStatus(0xFF)));
  }

  TEST_CASE("order_status_enum_to_fix") {
    REQUIRE(*get_order_status(Nexus::OrderStatus::NEW) ==
      FIX::OrdStatus(FIX::OrdStatus_NEW));
    REQUIRE(*get_order_status(Nexus::OrderStatus::FILLED) ==
      FIX::OrdStatus(FIX::OrdStatus_FILLED));
    REQUIRE(!get_order_status(static_cast<Nexus::OrderStatus>(-1)));
  }

  TEST_CASE("order_type_enum_to_fix") {
    REQUIRE(*get_order_type(Nexus::OrderType::LIMIT) ==
      FIX::OrdType(FIX::OrdType_LIMIT));
    REQUIRE(*get_order_type(Nexus::OrderType::MARKET) ==
      FIX::OrdType(FIX::OrdType_MARKET));
    REQUIRE(!get_order_type(static_cast<Nexus::OrderType>(-1)));
  }

  TEST_CASE("order_type_fix_to_enum") {
    REQUIRE(*get_order_type(FIX::OrdType(FIX::OrdType_LIMIT)) ==
      Nexus::OrderType::LIMIT);
    REQUIRE(*get_order_type(FIX::OrdType(FIX::OrdType_MARKET)) ==
      Nexus::OrderType::MARKET);
    REQUIRE(!get_order_type(FIX::OrdType(0xFF)));
  }

  TEST_CASE("side_enum_to_fix") {
    REQUIRE(*get_side(Nexus::Side::BID, false) == FIX::Side(FIX::Side_BUY));
    REQUIRE(*get_side(Nexus::Side::ASK, false) == FIX::Side(FIX::Side_SELL));
    REQUIRE(
      *get_side(Nexus::Side::ASK, true) == FIX::Side(FIX::Side_SELL_SHORT));
    REQUIRE(!get_side(static_cast<Nexus::Side>(-1), false));
  }

  TEST_CASE("time_in_force_enum_to_fix") {
    REQUIRE(*get_time_in_force_type(Nexus::TimeInForce::Type::DAY) ==
      FIX::TimeInForce(FIX::TimeInForce_DAY));
    REQUIRE(*get_time_in_force_type(Nexus::TimeInForce::Type::GTC) ==
      FIX::TimeInForce(FIX::TimeInForce_GOOD_TILL_CANCEL));
    REQUIRE(!get_time_in_force_type(static_cast<Nexus::TimeInForce::Type>(-1)));
  }
}
