#include <doctest/doctest.h>
#include "Nexus/TelemetryService/LocalTelemetryDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::TelemetryService;

TEST_SUITE("LocalTelemetryDataStore") {
  TEST_CASE("load_empty_events") {
    auto dataStore = LocalTelemetryDataStore();
    auto query = AccountQuery();
    query.SetIndex(DirectoryEntry::MakeAccount(123, "test"));
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto snapshot = dataStore.LoadTelemetryEvents(query);
    REQUIRE(snapshot.empty());
  }

  TEST_CASE("store_and_load") {
    auto dataStore = LocalTelemetryDataStore();
    auto data = JsonObject();
    data["size"] = [] {
      auto size = JsonObject();
      size["width"] = 200;
      size["height"] = 300;
      return size;
    }();
    auto event = TelemetryEvent("abcd", "spire.blotter.size",
      time_from_string("2021-10-23 13:01:12"), data);
    auto account = DirectoryEntry::MakeAccount(123, "test");
    auto value = SequencedValue(
      IndexedValue(event, account), Beam::Queries::Sequence(221));
    dataStore.Store(value);
    auto query = AccountQuery();
    query.SetIndex(account);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto snapshot = dataStore.LoadTelemetryEvents(query);
    REQUIRE(snapshot.size() == 1);
    REQUIRE(snapshot.front().GetValue() == *value);
    REQUIRE(snapshot.front().GetSequence() == value.GetSequence());
  }

  TEST_CASE("store_and_load_list") {
    auto dataStore = LocalTelemetryDataStore();
    auto dataA = JsonObject();
    dataA["size"] = [] {
      auto size = JsonObject();
      size["width"] = 200;
      size["height"] = 300;
      return size;
    }();
    auto account = DirectoryEntry::MakeAccount(123, "test");
    auto eventA = TelemetryEvent("abcd", "spire.blotter.size",
      time_from_string("2021-10-23 13:01:12"), dataA);
    auto events = std::vector<SequencedAccountTelemetryEvent>();
    events.push_back(SequencedValue(
      IndexedValue(eventA, account), Beam::Queries::Sequence(221)));
    auto dataB = JsonObject();
    dataB["size"] = [] {
      auto size = JsonObject();
      size["width"] = 220;
      size["height"] = 320;
      return size;
    }();
    auto eventB = TelemetryEvent("abcd", "spire.blotter.size",
      time_from_string("2021-10-23 13:01:13"), dataB);
    events.push_back(SequencedValue(
      IndexedValue(eventB, account), Beam::Queries::Sequence(222)));
    dataStore.Store(events);
    auto query = AccountQuery();
    query.SetIndex(account);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto snapshot = dataStore.LoadTelemetryEvents(query);
    REQUIRE(snapshot.size() == 2);
    REQUIRE(snapshot[0].GetValue() == *events[0]);
    REQUIRE(snapshot[0].GetSequence() == events[0].GetSequence());
    REQUIRE(snapshot[1].GetValue() == *events[1]);
    REQUIRE(snapshot[1].GetSequence() == events[1].GetSequence());
    auto allEvents = dataStore.LoadTelemetryEvents();
    REQUIRE(allEvents == events);
  }
}
