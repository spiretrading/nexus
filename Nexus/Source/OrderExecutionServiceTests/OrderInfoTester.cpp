#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/OrderInfo.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  auto make_test_order_fields() {
    auto account = DirectoryEntry::make_account(123, "test");
    auto security = Security("TST", TSX);
    auto currency = CAD;
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    return make_limit_order_fields(
      account, security, currency, side, destination, quantity, price);
  }
}

TEST_SUITE("OrderInfo") {
  TEST_CASE("default_constructor") {
    auto info = OrderInfo();
    REQUIRE(info.m_fields == OrderFields());
    REQUIRE(info.m_submission_account == DirectoryEntry());
    REQUIRE(info.m_id == static_cast<OrderId>(-1));
    REQUIRE(!info.m_shorting_flag);
    REQUIRE(info.m_timestamp == ptime());
  }

  TEST_CASE("constructor") {
    auto fields = make_test_order_fields();
    auto submission_account = DirectoryEntry::make_account(456, "submit");
    auto id = OrderId(123);
    auto shorting_flag = true;
    auto timestamp = time_from_string("2024-05-21 01:02:03");
    auto info =
      OrderInfo(fields, submission_account, id, shorting_flag, timestamp);
    REQUIRE(info.m_fields == fields);
    REQUIRE(info.m_submission_account == submission_account);
    REQUIRE(info.m_id == id);
    REQUIRE(info.m_shorting_flag == shorting_flag);
    REQUIRE(info.m_timestamp == timestamp);
  }

  TEST_CASE("constructor_without_submission_account") {
    auto fields = make_test_order_fields();
    auto id = OrderId(123);
    auto shorting_flag = true;
    auto timestamp = time_from_string("2024-05-21 01:02:03");
    auto info = OrderInfo(fields, id, shorting_flag, timestamp);
    REQUIRE(info.m_fields == fields);
    REQUIRE(info.m_submission_account == fields.m_account);
    REQUIRE(info.m_id == id);
    REQUIRE(info.m_shorting_flag == shorting_flag);
    REQUIRE(info.m_timestamp == timestamp);
  }

  TEST_CASE("constructor_without_shorting_flag") {
    auto fields = make_test_order_fields();
    auto id = OrderId(123);
    auto timestamp = time_from_string("2024-05-21 01:02:03");
    auto info = OrderInfo(fields, id, timestamp);
    REQUIRE(info.m_fields == fields);
    REQUIRE(info.m_submission_account == fields.m_account);
    REQUIRE(info.m_id == id);
    REQUIRE(!info.m_shorting_flag);
    REQUIRE(info.m_timestamp == timestamp);
  }

  TEST_CASE("stream") {
    auto fields = make_test_order_fields();
    auto submission_account = DirectoryEntry::make_account(456, "submit");
    auto id = OrderId(123);
    auto shorting_flag = true;
    auto timestamp = time_from_string("2024-05-21 01:02:03");
    auto info =
      OrderInfo(fields, submission_account, id, shorting_flag, timestamp);
    REQUIRE(to_string(info) ==
      "(((ACCOUNT 123 test) TST.TSX CAD LIMIT BID TSX 100"
      " 1.00 DAY []) (ACCOUNT 456 submit) 123 1 2024-May-21 01:02:03)");
    test_round_trip_shuttle(info);
  }
}
