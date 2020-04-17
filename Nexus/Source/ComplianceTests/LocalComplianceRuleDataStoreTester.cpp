#include <doctest/doctest.h>
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::Compliance;

namespace {
  auto GetAccountA() {
    return DirectoryEntry::MakeAccount(10, "test_a");
  }

  auto GetAccountB() {
    return DirectoryEntry::MakeAccount(11, "test_b");
  }

  auto GetSchemaA() {
    return ComplianceRuleSchema("schema_a", {});
  }

  auto GetSchemaB() {
    return ComplianceRuleSchema("schema_b", {});
  }

  auto GetEntryA() {
    return ComplianceRuleEntry(123, GetAccountA(),
      ComplianceRuleEntry::State::ACTIVE, GetSchemaA());
  }

  auto GetEntryB() {
    return ComplianceRuleEntry(124, GetAccountB(),
      ComplianceRuleEntry::State::ACTIVE, GetSchemaB());
  }
}

TEST_SUITE("LocalComplianceRuleDataStore") {
  TEST_CASE("queries_by_id") {
    auto dataStore = LocalComplianceRuleDataStore();
    auto expectedEntry = GetEntryA();
    {
      auto entry = dataStore.LoadComplianceRuleEntry(expectedEntry.GetId());
      REQUIRE(!entry.is_initialized());
    }
    dataStore.Store(expectedEntry);
    {
      auto entry = dataStore.LoadComplianceRuleEntry(expectedEntry.GetId());
      REQUIRE(entry.is_initialized());
      REQUIRE(*entry == expectedEntry);
    }
    dataStore.Delete(expectedEntry.GetId());
    {
      auto entry = dataStore.LoadComplianceRuleEntry(expectedEntry.GetId());
      REQUIRE(!entry.is_initialized());
    }
  }

  TEST_CASE("loads_by_directory_entry") {
    auto dataStore = LocalComplianceRuleDataStore();
    {
      auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
      REQUIRE(result.empty());
    }
    auto entryA = GetEntryA();
    dataStore.Store(entryA);
    {
      auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
      REQUIRE(result.size() == 1);
      REQUIRE(result[0] == entryA);
    }
    {
      auto result = dataStore.LoadComplianceRuleEntries(GetAccountB());
      REQUIRE(result.empty());
    }
    auto entryB = GetEntryB();
    dataStore.Store(entryB);
    {
      auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
      REQUIRE(result.size() == 1);
      REQUIRE(result[0] == entryA);
    }
    {
      auto result = dataStore.LoadComplianceRuleEntries(GetAccountB());
      REQUIRE(result.size() == 1);
      REQUIRE(result[0] == entryB);
    }
    auto entryC = ComplianceRuleEntry{entryA.GetId(), GetAccountB(),
      ComplianceRuleEntry::State::ACTIVE, entryA.GetSchema()};
    dataStore.Store(entryC);
    {
      auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
      REQUIRE(result.empty());
    }
    {
      auto result = dataStore.LoadComplianceRuleEntries(GetAccountB());
      REQUIRE(result.size() == 2);
      REQUIRE(result[0] == entryB);
      REQUIRE(result[1] == entryC);
    }
  }

  TEST_CASE("delete_by_directory_entry") {
    auto dataStore = LocalComplianceRuleDataStore();
    auto entryA = GetEntryA();
    dataStore.Store(entryA);
    auto entryB = ComplianceRuleEntry{124, entryA.GetDirectoryEntry(),
      ComplianceRuleEntry::State::ACTIVE, GetEntryB().GetSchema()};
    dataStore.Store(entryB);
    {
      auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
      REQUIRE(result.size() == 2);
      REQUIRE(result[0] == entryA);
      REQUIRE(result[1] == entryB);
    }
    dataStore.Delete(entryB.GetId());
    {
      auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
      REQUIRE(result.size() == 1);
      REQUIRE(result[0] == entryA);
    }
    dataStore.Delete(entryA.GetId());
    {
      auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
      REQUIRE(result.empty());
    }
  }
}
