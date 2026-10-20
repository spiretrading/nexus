#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Venues;

namespace {
  auto make_test_order_info() {
    auto fields = make_limit_order_fields(
      DirectoryEntry::make_account(123, "test"), parse_ticker("TST.TSX"), CAD,
      Side::BID, Destinations::TSX, 100, Money::ONE);
    auto submission_account = DirectoryEntry::make_account(456, "submit");
    return OrderInfo(fields, submission_account, 123, true,
      time_from_string("2024-05-21 01:02:03"));
  }
}

TEST_SUITE("OrderRecord") {
  TEST_CASE("format_databases") {
    auto currencies = CurrencyDatabase();
    auto currency = CurrencyDatabase::Entry();
    currency.m_id = CAD;
    currency.m_code = "CUS";
    currencies.add(currency);
    auto venues = VenueDatabase();
    auto venue = VenueDatabase::Entry();
    venue.m_venue = TSX;
    venue.m_display_name = "VENUE";
    venues.add(venue);
    auto locale = make_locale(currencies, venues);
    auto info = make_test_order_info();
    auto fields = std::string(
      "((ACCOUNT 123 test) TST.VENUE CUS LIMIT BID TSX 100 1.00 DAY [])");
    auto expected = std::format(
      "({} (ACCOUNT 456 submit) 123 true 2024-May-21 01:02:03)", fields);
    auto record = OrderRecord(info, {});
    auto test = [&] (const auto& value, const std::string& expected) {
      REQUIRE(std::format(locale, "{}", value) == expected);
      auto output = std::ostringstream();
      output.imbue(locale);
      output << value;
      REQUIRE(output.str() == expected);
      output.str("");
      output.imbue(std::locale::classic());
      output << currencies << venues << value;
      REQUIRE(output.str() == expected);
    };
    test(info.m_fields, fields);
    test(info, expected);
    test(record, std::format("({} [])", expected));
    auto indexed = IndexedValue(record, parse_ticker("TST.TSX"));
    auto indexed_text = std::format("(TST.VENUE ({} []))", expected);
    REQUIRE(std::format(locale, "{}", indexed) == indexed_text);
    auto output = std::ostringstream();
    output.imbue(locale);
    output << indexed;
    REQUIRE(output.str() == indexed_text);
    output.str("");
    output << CURRENCIES << VENUES << record;
    REQUIRE(output.str() == to_string(record));
  }

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
    auto expected = std::string(
      "((((ACCOUNT 123 test) TST.TSX CAD LIMIT BID TSX 100 1.00 DAY []) "
      "(ACCOUNT 456 submit) 123 true 2024-May-21 01:02:03) "
      "[(123 2024-May-21 01:02:04 1 NEW 0 0.00   0.00 0.00 0.00  []), "
      "(123 2024-May-21 01:02:05 2 PARTIALLY_FILLED 50 1.00 A  0.00 0.00 0.00  "
      "[])])");
    REQUIRE(to_string(record) == expected);
    REQUIRE(std::format("{}", record) == expected);
    REQUIRE(std::format("{:*>{}}", record, expected.size() + 2) ==
      "**" + expected);
    REQUIRE(std::format("{:.5}", record) == expected.substr(0, 5));
    test_round_trip_shuttle(record);
  }
}
