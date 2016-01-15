#include "Nexus/ComplianceTests/LocalComplianceRuleDataStoreTester.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::Compliance::Tests;
using namespace std;

namespace {
  DirectoryEntry GetAccountA() {
    DirectoryEntry account{DirectoryEntry::Type::ACCOUNT, 10, "test_a"};
    return account;
  }

  DirectoryEntry GetAccountB() {
    DirectoryEntry account{DirectoryEntry::Type::ACCOUNT, 11, "test_b"};
    return account;
  }

  ComplianceRuleSchema GetSchemaA() {
    ComplianceRuleSchema schema{"schema_a", {}};
    return schema;
  }

  ComplianceRuleSchema GetSchemaB() {
    ComplianceRuleSchema schema{"schema_b", {}};
    return schema;
  }

  ComplianceRuleEntry GetEntryA() {
    ComplianceRuleEntry expectedEntry{123, GetAccountA(),
      ComplianceRuleEntry::State::ACTIVE, GetSchemaA()};
    return expectedEntry;
  }

  ComplianceRuleEntry GetEntryB() {
    ComplianceRuleEntry expectedEntry{124, GetAccountB(),
      ComplianceRuleEntry::State::ACTIVE, GetSchemaB()};
    return expectedEntry;
  }
}

void LocalComplianceRuleDataStoreTester::TestQueriesById() {
  LocalComplianceRuleDataStore dataStore;
  auto expectedEntry = GetEntryA();
  {
    auto entry = dataStore.LoadComplianceRuleEntry(expectedEntry.GetId());
    CPPUNIT_ASSERT(!entry.is_initialized());
  }
  dataStore.Store(expectedEntry);
  {
    auto entry = dataStore.LoadComplianceRuleEntry(expectedEntry.GetId());
    CPPUNIT_ASSERT(entry.is_initialized());
    CPPUNIT_ASSERT(*entry == expectedEntry);
  }
  dataStore.Delete(expectedEntry.GetId());
  {
    auto entry = dataStore.LoadComplianceRuleEntry(expectedEntry.GetId());
    CPPUNIT_ASSERT(!entry.is_initialized());
  }
}

void LocalComplianceRuleDataStoreTester::TestLoadsByDirectoryEntry() {
  LocalComplianceRuleDataStore dataStore;
  {
    auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
    CPPUNIT_ASSERT(result.empty());
  }
  auto entryA = GetEntryA();
  dataStore.Store(entryA);
  {
    auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT(result[0] == entryA);
  }
  {
    auto result = dataStore.LoadComplianceRuleEntries(GetAccountB());
    CPPUNIT_ASSERT(result.empty());
  }
  auto entryB = GetEntryB();
  dataStore.Store(entryB);
  {
    auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT(result[0] == entryA);
  }
  {
    auto result = dataStore.LoadComplianceRuleEntries(GetAccountB());
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT(result[0] == entryB);
  }
  auto entryC = ComplianceRuleEntry{entryA.GetId(), GetAccountB(),
    ComplianceRuleEntry::State::ACTIVE, entryA.GetSchema()};
  dataStore.Store(entryC);
  {
    auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
    CPPUNIT_ASSERT(result.empty());
  }
  {
    auto result = dataStore.LoadComplianceRuleEntries(GetAccountB());
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT(result[0] == entryB);
    CPPUNIT_ASSERT(result[1] == entryC);
  }
}

void LocalComplianceRuleDataStoreTester::TestDeleteByDirectoryEntry() {
  LocalComplianceRuleDataStore dataStore;
  auto entryA = GetEntryA();
  dataStore.Store(entryA);
  auto entryB = ComplianceRuleEntry{124, entryA.GetDirectoryEntry(),
    ComplianceRuleEntry::State::ACTIVE, GetEntryB().GetSchema()};
  dataStore.Store(entryB);
  {
    auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT(result[0] == entryA);
    CPPUNIT_ASSERT(result[1] == entryB);
  }
  dataStore.Delete(entryB.GetId());
  {
    auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT(result[0] == entryA);
  }
  dataStore.Delete(entryA.GetId());
  {
    auto result = dataStore.LoadComplianceRuleEntries(GetAccountA());
    CPPUNIT_ASSERT(result.empty());
  }
}
