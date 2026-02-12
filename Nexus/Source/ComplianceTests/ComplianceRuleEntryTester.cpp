#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;

TEST_SUITE("compliance_rule_entry") {
  TEST_CASE("constructor") {
    auto directory_entry = DirectoryEntry::make_account(321, "alpha");
    auto parameter = ComplianceParameter("amount", 100.0);
    auto schema = ComplianceRuleSchema("limit_rule", std::vector{parameter});
    auto entry = ComplianceRuleEntry(
      99, directory_entry, ComplianceRuleEntry::State::PASSIVE, schema);
    REQUIRE(entry.get_id() == 99);
    REQUIRE(entry.get_directory_entry() == directory_entry);
    REQUIRE(entry.get_state() == ComplianceRuleEntry::State::PASSIVE);
    REQUIRE(entry.get_schema() == schema);
    entry.set_state(ComplianceRuleEntry::State::DISABLED);
    REQUIRE(entry.get_state() == ComplianceRuleEntry::State::DISABLED);
  }

  TEST_CASE("stream") {
    auto directory_entry = DirectoryEntry::make_account(123, "test");
    auto parameter = ComplianceParameter("quantity", true);
    auto schema = ComplianceRuleSchema("rule_name", std::vector{parameter});
    auto entry = ComplianceRuleEntry(
      42, directory_entry, ComplianceRuleEntry::State::ACTIVE, schema);
    REQUIRE(to_string(entry) ==
      "(42 (ACCOUNT 123 test) ACTIVE (rule_name [(quantity 1)]))");
    test_round_trip_shuttle(entry);
  }
}
