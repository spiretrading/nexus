#include <doctest/doctest.h>
#include "Nexus/FixUtilities/FixOrder.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

namespace {
  auto make_order_info() {
    auto fields = make_limit_order_fields(
      Security("TST", TSX), Side::BID, "TSX", 100, Money::ONE);
    return OrderInfo(fields, 123, time_from_string("2024-05-21 00:00:10.000"));
  }

  auto make_order_record() {
    auto info = make_order_info();
    auto report1 = ExecutionReport(123, info.m_timestamp);
    auto report2 = make_update(report1, OrderStatus::NEW, info.m_timestamp);
    return OrderRecord(info, {report1, report2});
  }
}

TEST_SUITE("FixOrder") {
  TEST_CASE("construct_from_order_info") {
    auto info = make_order_info();
    auto fix_side = FIX::Side(FIX::Side_BUY);
    auto order = FixOrder(info, fix_side);
    REQUIRE(order.get_info() == info);
    REQUIRE(order.get_side() == fix_side);
    REQUIRE(order.get_symbol() == info.m_fields.m_security.get_symbol());
  }

  TEST_CASE("construct_from_order_record") {
    auto record = make_order_record();
    auto fix_side = FIX::Side(FIX::Side_BUY);
    auto order = FixOrder(record, fix_side);
    REQUIRE(order.get_info() == record.m_info);
    REQUIRE(order.get_side() == fix_side);
    REQUIRE(order.get_symbol() ==
      record.m_info.m_fields.m_security.get_symbol());
  }

  TEST_CASE("cancel_id_increments") {
    auto info = make_order_info();
    auto fix_side = FIX::Side(FIX::Side_BUY);
    auto order = FixOrder(info, fix_side);
    auto id1 = order.get_next_cancel_id();
    auto id2 = order.get_next_cancel_id();
    REQUIRE(id1 != id2);
  }
}
