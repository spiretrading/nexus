#ifndef NEXUS_COMPLIANCE_RULE_DATA_STORE_TEST_SUITE_HPP
#define NEXUS_COMPLIANCE_RULE_DATA_STORE_TEST_SUITE_HPP
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"

namespace Nexus::Tests {
  TEST_CASE_TEMPLATE_DEFINE(
      "ComplianceRuleDataStore", T, ComplianceRuleDataStoreTestSuite) {
    using namespace Beam;
    using namespace boost;
    using namespace boost::posix_time;
    using namespace Nexus;

    auto data_store = T()();

    SUBCASE("store_and_load_single_entry") {
      auto directory_entry = DirectoryEntry::make_account(1, "alice");
      auto schema = ComplianceRuleSchema("test_rule", {});
      auto entry = ComplianceRuleEntry(
        100, directory_entry, ComplianceRuleEntry::State::ACTIVE, schema);
      data_store.store(entry);
      auto loaded_entry = data_store.load_compliance_rule_entry(100);
      REQUIRE(loaded_entry);
      REQUIRE(loaded_entry->get_id() == 100);
      REQUIRE(loaded_entry->get_directory_entry() == directory_entry);
      REQUIRE(loaded_entry->get_state() == ComplianceRuleEntry::State::ACTIVE);
      REQUIRE(loaded_entry->get_schema() == schema);
      auto all_entries = data_store.load_all_compliance_rule_entries();
      REQUIRE(all_entries.size() == 1);
      REQUIRE(all_entries.front().get_id() == 100);
    }

    SUBCASE("store_and_load_multiple_entries") {
      auto directory_entry1 = DirectoryEntry::make_account(1, "alice");
      auto directory_entry2 = DirectoryEntry::make_account(2, "bob");
      auto schema1 = ComplianceRuleSchema("rule1", {});
      auto schema2 = ComplianceRuleSchema("rule2", {});
      auto entry1 = ComplianceRuleEntry(
        101, directory_entry1, ComplianceRuleEntry::State::ACTIVE, schema1);
      auto entry2 = ComplianceRuleEntry(
        102, directory_entry2, ComplianceRuleEntry::State::PASSIVE, schema2);
      data_store.store(entry1);
      data_store.store(entry2);
      auto loaded_entry1 = data_store.load_compliance_rule_entry(101);
      auto loaded_entry2 = data_store.load_compliance_rule_entry(102);
      REQUIRE(loaded_entry1);
      REQUIRE(loaded_entry2);
      REQUIRE(loaded_entry1->get_id() == 101);
      REQUIRE(loaded_entry2->get_id() == 102);
      auto all_entries = data_store.load_all_compliance_rule_entries();
      REQUIRE(all_entries.size() == 2);
      REQUIRE(
        (all_entries[0].get_id() == 101 || all_entries[1].get_id() == 101));
      REQUIRE(
        (all_entries[0].get_id() == 102 || all_entries[1].get_id() == 102));
    }

    SUBCASE("load_compliance_rule_entries_by_directory_entry") {
      auto directory_entry1 = DirectoryEntry::make_account(1, "alice");
      auto directory_entry2 = DirectoryEntry::make_account(2, "bob");
      auto schema1 = ComplianceRuleSchema("rule1", {});
      auto schema2 = ComplianceRuleSchema("rule2", {});
      auto entry1 = ComplianceRuleEntry(
        201, directory_entry1, ComplianceRuleEntry::State::ACTIVE, schema1);
      auto entry2 = ComplianceRuleEntry(
        202, directory_entry1, ComplianceRuleEntry::State::PASSIVE, schema2);
      auto entry3 = ComplianceRuleEntry(
        203, directory_entry2, ComplianceRuleEntry::State::DISABLED, schema1);
      data_store.store(entry1);
      data_store.store(entry2);
      data_store.store(entry3);
      auto alice_entries =
        data_store.load_compliance_rule_entries(directory_entry1);
      REQUIRE(alice_entries.size() == 2);
      REQUIRE(
        (alice_entries[0].get_id() == 201 || alice_entries[1].get_id() == 201));
      REQUIRE((alice_entries[0].get_id() == 202 ||
        alice_entries[1].get_id() == 202));
      auto bob_entries =
        data_store.load_compliance_rule_entries(directory_entry2);
      REQUIRE(bob_entries.size() == 1);
      REQUIRE(bob_entries[0].get_id() == 203);
    }

    SUBCASE("update_existing_entry") {
      auto directory_entry = DirectoryEntry::make_account(1, "alice");
      auto schema1 = ComplianceRuleSchema("rule1", {});
      auto schema2 = ComplianceRuleSchema("rule2", {});
      auto entry = ComplianceRuleEntry(
        301, directory_entry, ComplianceRuleEntry::State::ACTIVE, schema1);
      data_store.store(entry);
      auto updated_entry = ComplianceRuleEntry(
        301, directory_entry, ComplianceRuleEntry::State::DISABLED, schema2);
      data_store.store(updated_entry);
      auto loaded_entry = data_store.load_compliance_rule_entry(301);
      REQUIRE(loaded_entry);
      REQUIRE(loaded_entry->get_id() == 301);
      REQUIRE(
        loaded_entry->get_state() == ComplianceRuleEntry::State::DISABLED);
      REQUIRE(loaded_entry->get_schema() == schema2);
      auto all_entries = data_store.load_all_compliance_rule_entries();
      REQUIRE(all_entries.size() == 1);
      REQUIRE(all_entries.front().get_id() == 301);
      REQUIRE(all_entries.front().get_state() ==
        ComplianceRuleEntry::State::DISABLED);
    }

    SUBCASE("remove_entry") {
      auto directory_entry = DirectoryEntry::make_account(1, "alice");
      auto schema = ComplianceRuleSchema("rule1", {});
      auto entry = ComplianceRuleEntry(
        401, directory_entry, ComplianceRuleEntry::State::ACTIVE, schema);
      data_store.store(entry);
      auto loaded_entry = data_store.load_compliance_rule_entry(401);
      REQUIRE(loaded_entry);
      data_store.remove(401);
      auto removed_entry = data_store.load_compliance_rule_entry(401);
      REQUIRE(!removed_entry);
      auto all_entries = data_store.load_all_compliance_rule_entries();
      REQUIRE(all_entries.empty());
      auto directory_entries =
        data_store.load_compliance_rule_entries(directory_entry);
      REQUIRE(directory_entries.empty());
    }

    SUBCASE("load_next_compliance_rule_entry_id") {
      REQUIRE(data_store.load_next_compliance_rule_entry_id() == 1);
      auto directory_entry = DirectoryEntry::make_account(1, "alice");
      auto schema = ComplianceRuleSchema("rule", {});
      data_store.store(ComplianceRuleEntry(
        10, directory_entry, ComplianceRuleEntry::State::ACTIVE, schema));
      data_store.store(ComplianceRuleEntry(
        5, directory_entry, ComplianceRuleEntry::State::ACTIVE, schema));
      data_store.store(ComplianceRuleEntry(
        20, directory_entry, ComplianceRuleEntry::State::ACTIVE, schema));
      REQUIRE(data_store.load_next_compliance_rule_entry_id() == 21);
    }

    SUBCASE("store_and_ignore_violation_record") {
      auto directory_entry = DirectoryEntry::make_account(1, "alice");
      auto schema = ComplianceRuleSchema("rule", {});
      auto entry = ComplianceRuleEntry(
        1, directory_entry, ComplianceRuleEntry::State::ACTIVE, schema);
      data_store.store(entry);
      auto violation = ComplianceRuleViolationRecord();
      violation.m_account = directory_entry;
      violation.m_order_id = 123;
      violation.m_rule_id = 1;
      violation.m_schema_name = "rule";
      violation.m_reason = "test";
      violation.m_timestamp = time_from_string("2024-07-25 12:00:00");
      data_store.store(violation);
      auto loaded_entry = data_store.load_compliance_rule_entry(1);
      REQUIRE(loaded_entry);
      REQUIRE(loaded_entry->get_id() == 1);
      auto all_entries = data_store.load_all_compliance_rule_entries();
      REQUIRE(all_entries.size() == 1);
    }

    SUBCASE("load_nonexistent_entry") {
      auto loaded_entry = data_store.load_compliance_rule_entry(999);
      REQUIRE(!loaded_entry);
      auto directory_entry = DirectoryEntry::make_account(1, "alice");
      auto schema = ComplianceRuleSchema("rule", {});
      data_store.store(ComplianceRuleEntry(
        2, directory_entry, ComplianceRuleEntry::State::ACTIVE, schema));
      auto missing_entry = data_store.load_compliance_rule_entry(3);
      REQUIRE(!missing_entry);
    }
  }
}

#endif
