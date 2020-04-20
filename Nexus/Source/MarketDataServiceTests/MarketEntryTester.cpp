#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/MarketEntry.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;

namespace {
  const auto TEST_SECURITY = Security("TST", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
}

TEST_SUITE("MarketEntry") {
  TEST_CASE("publish_order_imbalance") {
    auto initialSequences = MarketEntry::InitialSequences();
    auto entry = MarketEntry(DefaultMarkets::NASDAQ(), initialSequences);
    auto orderImbalanceA = OrderImbalance(TEST_SECURITY, Side::BID, 1000,
      Money::ONE, second_clock::universal_time());
    auto sequencedOrderImbalanceA = entry.PublishOrderImbalance(
      orderImbalanceA, 0);
    REQUIRE(sequencedOrderImbalanceA.is_initialized());
    REQUIRE(sequencedOrderImbalanceA->GetSequence() >=
      Beam::Queries::Sequence(0));
    REQUIRE((*sequencedOrderImbalanceA)->GetIndex() ==
      DefaultMarkets::NASDAQ());
    REQUIRE(orderImbalanceA == **sequencedOrderImbalanceA);
    auto orderImbalanceB = OrderImbalance(TEST_SECURITY, Side::ASK, 2000,
      Money::CENT, second_clock::universal_time());
    auto sequencedOrderImbalanceB = entry.PublishOrderImbalance(orderImbalanceB,
      0);
    REQUIRE(sequencedOrderImbalanceB.is_initialized());
    REQUIRE(sequencedOrderImbalanceB->GetSequence() >
      sequencedOrderImbalanceA->GetSequence());
    REQUIRE((*sequencedOrderImbalanceB)->GetIndex() ==
      DefaultMarkets::NASDAQ());
    REQUIRE(orderImbalanceB == **sequencedOrderImbalanceB);
  }
}
