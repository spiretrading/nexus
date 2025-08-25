#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/VenueEntry.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("VenueEntry") {
  TEST_CASE("publish_order_imbalance") {
    auto initial_sequences = VenueEntry::InitialSequences();
    initial_sequences.m_next_order_imbalance_sequence =
      Beam::Queries::Sequence(10);
    auto venue = TSX;
    auto entry = VenueEntry(venue, initial_sequences);
    auto imbalance = OrderImbalance(Security("TST", venue), Side::BID, 100,
      Money::ONE, time_from_string("2024-07-10 00:00:01"));
    auto result1 = entry.publish(imbalance, 1);
    REQUIRE(result1);
    REQUIRE(**result1 == imbalance);
    REQUIRE(result1->GetValue().GetIndex() == venue);
    REQUIRE(result1->GetSequence() == Beam::Queries::Sequence(10));
    auto imbalance2 = OrderImbalance(Security("TST", venue), Side::ASK, 200,
      Money::ONE, time_from_string("2024-07-10 00:00:02"));
    auto result2 = entry.publish(imbalance2, 1);
    REQUIRE(result2);
    REQUIRE(**result2 == imbalance2);
    REQUIRE(result2->GetValue().GetIndex() == venue);
    REQUIRE(result2->GetSequence() == Beam::Queries::Sequence(11));
  }

  TEST_CASE("load_initial_sequences") {
    auto data_store = LocalHistoricalDataStore();
    data_store.store(SequencedValue(IndexedValue(
      OrderImbalance(Security("A", TSX), Side::BID, 100, Money::ONE,
        time_from_string("2024-07-10 12:00:00")), TSX),
      Beam::Queries::Sequence(5)));
    data_store.store(SequencedValue(IndexedValue(
      OrderImbalance(Security("B", TSX), Side::ASK, 200, Money::ONE,
        time_from_string("2024-07-10 12:00:01")), TSX),
      Beam::Queries::Sequence(3)));
    data_store.store(SequencedValue(IndexedValue(
      OrderImbalance(Security("C", TSX), Side::BID, 150, Money::ONE,
        time_from_string("2024-07-10 12:00:02")), TSX),
      Beam::Queries::Sequence(7)));
    data_store.store(SequencedValue(IndexedValue(
      OrderImbalance(Security("D", NYSE), Side::BID, 300, Money::ONE,
        time_from_string("2024-07-10 12:00:00")), NYSE),
      Beam::Queries::Sequence(15)));
    data_store.store(SequencedValue(IndexedValue(
      OrderImbalance(Security("E", NYSE), Side::ASK, 400, Money::ONE,
        time_from_string("2024-07-10 12:00:01")), NYSE),
      Beam::Queries::Sequence(18)));
    data_store.store(SequencedValue(IndexedValue(
      OrderImbalance(Security("F", NYSE), Side::BID, 500, Money::ONE,
        time_from_string("2024-07-10 12:00:02")), NYSE),
      Beam::Queries::Sequence(12)));
    auto tsx_sequences = load_initial_sequences(data_store, TSX);
    REQUIRE(tsx_sequences.m_next_order_imbalance_sequence ==
      Beam::Queries::Sequence(8));
    auto nyse_sequences = load_initial_sequences(data_store, NYSE);
    REQUIRE(nyse_sequences.m_next_order_imbalance_sequence ==
      Beam::Queries::Sequence(19));
    auto asx_sequences = load_initial_sequences(data_store, ASX);
    REQUIRE(asx_sequences.m_next_order_imbalance_sequence ==
      Beam::Queries::Sequence::First());
  }
}
