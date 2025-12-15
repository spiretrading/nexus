#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace boost::posix_time;

TEST_SUITE("ComplianceRuleViolationRecord") {
  TEST_CASE("stream") {
    auto record = ComplianceRuleViolationRecord();
    record.m_account = DirectoryEntry::make_account(42, "alice");
    record.m_order_id = 12345;
    record.m_rule_id = 67890;
    record.m_schema_name = "buying_power";
    record.m_reason = "Order exceeds available buying power.";
    record.m_timestamp = time_from_string("2024-07-25 15:30:00");
    REQUIRE(to_string(record) ==
      "((ACCOUNT 42 alice) 12345 67890 buying_power "
      "\"Order exceeds available buying power.\" 2024-Jul-25 15:30:00)");
    test_round_trip_shuttle(record);
  }
}
