#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;

TEST_SUITE("compliance_rule_entry") {
  TEST_CASE("constructor") {
    auto directory_entry = DirectoryEntry::MakeAccount(321, "alpha");
    auto parameter = ComplianceParameter("amount", ComplianceValue(100.0));
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

  TEST_CASE("state_stream") {
    auto stream = std::ostringstream();
    stream << ComplianceRuleEntry::State::ACTIVE;
    REQUIRE(stream.str() == "ACTIVE");
    stream.str("");
    stream << ComplianceRuleEntry::State::PASSIVE;
    REQUIRE(stream.str() == "PASSIVE");
    stream.str("");
    stream << ComplianceRuleEntry::State::DISABLED;
    REQUIRE(stream.str() == "DISABLED");
    stream.str("");
    stream << ComplianceRuleEntry::State::DELETED;
    REQUIRE(stream.str() == "DELETED");
  }

  TEST_CASE("stream") {
    auto directory_entry = DirectoryEntry::MakeAccount(123, "test");
    auto parameter = ComplianceParameter("quantity", true);
    auto schema = ComplianceRuleSchema("rule_name", std::vector{parameter});
    auto entry = ComplianceRuleEntry(
      42, directory_entry, ComplianceRuleEntry::State::ACTIVE, schema);
    auto stream = std::ostringstream();
    stream << entry;
    REQUIRE(stream.str() ==
      "(42 (ACCOUNT 123 test) ACTIVE (rule_name [(quantity 1)]))");
    SUBCASE("shuttle") {
      Beam::Serialization::Tests::TestRoundTripShuttle(entry);
    }
  }
}
