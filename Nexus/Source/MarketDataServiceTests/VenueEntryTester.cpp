#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/VenueEntry.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("VenueEntry") {
  TEST_CASE("publish_order_imbalance") {
    auto initial_sequences = VenueEntry::InitialSequences();
    initial_sequences.m_next_order_imbalance_sequence = Beam::Sequence(10);
    auto venue = TSX;
    auto entry = VenueEntry(venue, initial_sequences);
    auto imbalance = OrderImbalance(Ticker("TST", venue), Side::BID, 100,
      Money::ONE, time_from_string("2024-07-10 00:00:01"));
    auto result1 = entry.publish(imbalance, 1);
    REQUIRE(result1);
    REQUIRE(**result1 == imbalance);
    REQUIRE((*result1)->get_index() == venue);
    REQUIRE(result1->get_sequence() == Beam::Sequence(10));
    auto imbalance2 = OrderImbalance(Ticker("TST", venue), Side::ASK, 200,
      Money::ONE, time_from_string("2024-07-10 00:00:02"));
    auto result2 = entry.publish(imbalance2, 1);
    REQUIRE(result2);
    REQUIRE(**result2 == imbalance2);
    REQUIRE((*result2)->get_index() == venue);
    REQUIRE(result2->get_sequence() == Beam::Sequence(11));
  }

  TEST_CASE("load_initial_sequences") {
    auto data_store = LocalHistoricalDataStore();
    data_store.store(SequencedValue(IndexedValue(
      OrderImbalance(Ticker("A", TSX), Side::BID, 100, Money::ONE,
        time_from_string("2024-07-10 12:00:00")), TSX), Beam::Sequence(5)));
    data_store.store(SequencedValue(IndexedValue(
      OrderImbalance(Ticker("B", TSX), Side::ASK, 200, Money::ONE,
        time_from_string("2024-07-10 12:00:01")), TSX), Beam::Sequence(3)));
    data_store.store(SequencedValue(IndexedValue(
      OrderImbalance(Ticker("C", TSX), Side::BID, 150, Money::ONE,
        time_from_string("2024-07-10 12:00:02")), TSX), Beam::Sequence(7)));
    data_store.store(SequencedValue(IndexedValue(
      OrderImbalance(Ticker("D", ASX), Side::BID, 300, Money::ONE,
        time_from_string("2024-07-10 12:00:00")), ASX), Beam::Sequence(15)));
    data_store.store(SequencedValue(IndexedValue(
      OrderImbalance(Ticker("E", ASX), Side::ASK, 400, Money::ONE,
        time_from_string("2024-07-10 12:00:01")), ASX), Beam::Sequence(18)));
    data_store.store(SequencedValue(IndexedValue(
      OrderImbalance(Ticker("F", ASX), Side::BID, 500, Money::ONE,
        time_from_string("2024-07-10 12:00:02")), ASX), Beam::Sequence(12)));
    auto tsx_sequences = load_initial_sequences(data_store, TSX);
    REQUIRE(tsx_sequences.m_next_order_imbalance_sequence == Beam::Sequence(8));
    auto asx_sequences = load_initial_sequences(data_store, ASX);
    REQUIRE(
      asx_sequences.m_next_order_imbalance_sequence == Beam::Sequence(19));
    auto tsxv_sequences = load_initial_sequences(data_store, TSXV);
    REQUIRE(
      tsxv_sequences.m_next_order_imbalance_sequence == Beam::Sequence::FIRST);
  }
}
