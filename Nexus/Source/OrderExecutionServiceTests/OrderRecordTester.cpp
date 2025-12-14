#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/OrderRecord.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  auto make_test_order_info() {
    auto fields = make_limit_order_fields(
      DirectoryEntry::make_account(123, "test"), Security("TST", TSX), CAD,
      Side::BID, DefaultDestinations::TSX, 100, Money::ONE);
    auto submission_account = DirectoryEntry::make_account(456, "submit");
    return OrderInfo(fields, submission_account, 123, true,
      time_from_string("2024-05-21 01:02:03"));
  }
}

TEST_SUITE("OrderRecord") {
  TEST_CASE("default_constructor") {
    auto record = OrderRecord();
    REQUIRE(record.m_info == OrderInfo());
    REQUIRE(record.m_execution_reports.empty());
  }

  TEST_CASE("constructor") {
    auto info = make_test_order_info();
    auto reports = std::vector<ExecutionReport>();
    reports.push_back(
      ExecutionReport(info.m_id, time_from_string("2024-05-21 01:02:04")));
    auto record = OrderRecord(info, reports);
    REQUIRE(record.m_info == info);
    REQUIRE(record.m_execution_reports == reports);
  }

  TEST_CASE("stream") {
    auto info = make_test_order_info();
    auto reports = std::vector<ExecutionReport>();
    auto report1 =
      ExecutionReport(info.m_id, time_from_string("2024-05-21 01:02:04"));
    report1.m_sequence = 1;
    report1.m_status = OrderStatus::NEW;
    reports.push_back(report1);
    auto report2 =
      ExecutionReport(info.m_id, time_from_string("2024-05-21 01:02:05"));
    report2.m_sequence = 2;
    report2.m_status = OrderStatus::PARTIALLY_FILLED;
    report2.m_last_quantity = 50;
    report2.m_last_price = Money::ONE;
    report2.m_liquidity_flag = "A";
    reports.push_back(report2);
    auto record = OrderRecord(info, reports);
    auto stream = std::stringstream();
    stream << record;
    REQUIRE(stream.str() ==
      "((((ACCOUNT 123 test) TST.TSX CAD LIMIT BID TSX 100 1.00 DAY []) "
      "(ACCOUNT 456 submit) 123 1 2024-May-21 01:02:03) "
      "[(123 2024-May-21 01:02:04 1 NEW 0 0.00   0.00 0.00 0.00  []), "
      "(123 2024-May-21 01:02:05 2 PARTIALLY_FILLED 50 1.00 A  0.00 0.00 0.00  "
      "[])])");
    test_round_trip_shuttle(record);
  }
}
