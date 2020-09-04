#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/RiskService/RiskClient.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace Nexus;
using namespace Nexus::RiskService;

namespace {
  struct Fixture {
    using TestRiskClient = RiskClient<TestServiceProtocolClientBuilder>;
    optional<TestServiceProtocolServer> m_server;
    optional<TestRiskClient> m_client;

    Fixture() {
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_server.emplace(serverConnection,
        factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
      RegisterRiskServices(Store(m_server->GetSlots()));
      RegisterRiskMessages(Store(m_server->GetSlots()));
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
            "test", *serverConnection);
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_client.emplace(builder);
    }
  };
}

TEST_SUITE("RiskClient") {
  TEST_CASE_FIXTURE(Fixture, "load_inventory_snapshot_no_fail") {
    auto receivedAccount = optional<DirectoryEntry>();
    LoadInventorySnapshotService::AddSlot(Store(m_server->GetSlots()),
      [&] (auto& client, const auto& account) {
        receivedAccount.emplace(account);
        auto snapshot = InventorySnapshot();
        snapshot.m_sequence = Beam::Queries::Sequence(123);
        snapshot.m_excludedOrders.push_back(221);
        return snapshot;
      });
    auto sentAccount = DirectoryEntry::MakeAccount(438, "kranar");
    auto receivedSnapshot = m_client->LoadInventorySnapshot(sentAccount);
    REQUIRE(receivedAccount == sentAccount);
    REQUIRE(receivedSnapshot.m_sequence == Beam::Queries::Sequence(123));
    REQUIRE(receivedSnapshot.m_excludedOrders.size() == 1);
    REQUIRE(receivedSnapshot.m_excludedOrders[0] == 221);
    REQUIRE(receivedSnapshot.m_inventories.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "load_inventory_snapshot_fail") {
    LoadInventorySnapshotService::AddSlot(Store(m_server->GetSlots()),
      [&] (auto& client, const auto& account) -> InventorySnapshot {
        throw ServiceRequestException("Insufficient permissions.");
      });
    auto sentAccount = DirectoryEntry::MakeAccount(438, "kranar");
    REQUIRE_THROWS_AS_MESSAGE(m_client->LoadInventorySnapshot(sentAccount),
      ServiceRequestException, "Insufficient permissions.");
  }

  TEST_CASE_FIXTURE(Fixture, "reset_no_fail") {
    auto receivedRegion = optional<Region>();
    ResetRegionService::AddSlot(Store(m_server->GetSlots()),
      [&] (auto& client, const auto& region) {
        receivedRegion.emplace(region);
      });
    auto sentRegion = Region(Security("XIU", DefaultMarkets::TSX(),
      DefaultCountries::CA()));
    m_client->Reset(sentRegion);
    REQUIRE((receivedRegion == sentRegion));
  }

  TEST_CASE_FIXTURE(Fixture, "reset_fail") {
    ResetRegionService::AddSlot(Store(m_server->GetSlots()),
      [&] (auto& client, const auto& region) {
        throw ServiceRequestException("Insufficient permissions.");
      });
    auto sentRegion = Region(Security("XIU", DefaultMarkets::TSX(),
      DefaultCountries::CA()));
    REQUIRE_THROWS_AS_MESSAGE(m_client->Reset(sentRegion),
      ServiceRequestException, "Insufficient permissions.");
  }
}
