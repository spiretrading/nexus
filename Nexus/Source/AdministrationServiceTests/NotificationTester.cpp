#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/Notification.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  const auto& get_test_notification() {
    static auto notification = Notification(
      "abc-123", DirectoryEntry::make_account(100, "account"),
      "Your request has been approved.", "{\"request_id\":42}",
      Notification::Category::ACCOUNT_MODIFICATION,
      time_from_string("2026-04-21 12:00:00"), false);
    return notification;
  }
}

TEST_SUITE("Notification") {
  TEST_CASE("shuttle") {
    test_round_trip_shuttle(get_test_notification(), [] (const auto& received) {
      auto& expected = get_test_notification();
      REQUIRE(received.m_id == expected.m_id);
      REQUIRE(received.m_account == expected.m_account);
      REQUIRE(received.m_description == expected.m_description);
      REQUIRE(received.m_data == expected.m_data);
      REQUIRE(received.m_category == expected.m_category);
      REQUIRE(received.m_timestamp == expected.m_timestamp);
      REQUIRE(received.m_is_read == expected.m_is_read);
    });
  }

  TEST_CASE("shuttle_read_notification") {
    auto notification = Notification(
      "def-456", DirectoryEntry::make_account(200, "user"),
      "Risk parameters updated.", "", Notification::Category::REPORT,
      time_from_string("2026-04-20 09:30:00"), true);
    test_round_trip_shuttle(notification, [] (const auto& received) {
      REQUIRE(received.m_id == "def-456");
      REQUIRE(received.m_category == Notification::Category::REPORT);
      REQUIRE(received.m_is_read);
    });
  }

  TEST_CASE("stream") {
    auto& notification = get_test_notification();
    auto expected = std::string(
      R"((abc-123 (ACCOUNT 100 account) "Your request has been approved.")"
      R"( "{"request_id":42}" ACCOUNT_MODIFICATION)"
      R"( 2026-Apr-21 12:00:00 0))");
    REQUIRE(to_string(notification) == expected);
  }

  TEST_CASE("stream_category") {
    auto out = std::ostringstream();
    out << Notification::Category::ACCOUNT_MODIFICATION;
    REQUIRE(out.str() == "ACCOUNT_MODIFICATION");
    out.str("");
    out << Notification::Category::REPORT;
    REQUIRE(out.str() == "REPORT");
  }
}
