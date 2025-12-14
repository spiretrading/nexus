#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/SecurityEntry.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("SecurityEntry") {
  TEST_CASE("publish_bbo_quote") {
    auto initial_sequences = SecurityEntry::InitialSequences();
    initial_sequences.m_next_bbo_quote_sequence = Beam::Sequence(20);
    auto security = Security("TST", TSX);
    auto entry = SecurityEntry(security, DEFAULT_VENUES,
      get_default_time_zone_database(), Money::ONE, initial_sequences);
    auto bbo_quote =
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-11 13:00:00"));
    auto result1 = entry.publish(bbo_quote, 1);
    REQUIRE(result1);
    REQUIRE(***result1 == bbo_quote);
    REQUIRE((*result1)->get_index() == security);
    REQUIRE(result1->get_sequence() == Beam::Sequence(20));
    REQUIRE(**entry.get_bbo_quote() == bbo_quote);
    REQUIRE(entry.get_bbo_quote().get_sequence() == Beam::Sequence(20));
    auto bbo_quote2 =
      BboQuote(make_bid(Money::CENT, 300), make_ask(2 * Money::CENT, 400),
        time_from_string("2024-07-11 13:00:01"));
    auto result2 = entry.publish(bbo_quote2, 1);
    REQUIRE(result2);
    REQUIRE(***result2 == bbo_quote2);
    REQUIRE((*result2)->get_index() == security);
    REQUIRE(result2->get_sequence() == Beam::Sequence(21));
    REQUIRE(**entry.get_bbo_quote() == bbo_quote2);
    REQUIRE(entry.get_bbo_quote().get_sequence() == Beam::Sequence(21));
  }

  TEST_CASE("publish_book_quote") {
    auto initial_sequences = SecurityEntry::InitialSequences();
    initial_sequences.m_next_book_quote_sequence = Beam::Sequence(50);
    auto security = Security("TST", TSX);
    auto entry = SecurityEntry(security, DEFAULT_VENUES,
      get_default_time_zone_database(), Money::ONE, initial_sequences);
    auto bid1 = BookQuote("MP1", false, TSX, make_bid(Money::CENT, 100),
      time_from_string("2024-07-11 13:00:00"));
    auto result_bid1 = entry.publish(bid1, 1);
    REQUIRE(result_bid1);
    REQUIRE(result_bid1->get_sequence() == Beam::Sequence(50));
    auto ask1 = BookQuote("MP1", false, TSX, make_ask(2 * Money::CENT, 200),
      time_from_string("2024-07-11 13:00:01"));
    auto result_ask1 = entry.publish(ask1, 1);
    REQUIRE(result_ask1);
    REQUIRE(result_ask1->get_sequence() == Beam::Sequence(51));
    auto bid2 = BookQuote("MP2", false, TSX, make_bid(Money::CENT, 300),
      time_from_string("2024-07-11 13:00:02"));
    auto result_bid2 = entry.publish(bid2, 1);
    REQUIRE(result_bid2);
    REQUIRE(result_bid2->get_sequence() == Beam::Sequence(52));
    auto ask2 = BookQuote("MP2", false, TSX, make_ask(2 * Money::CENT, 400),
      time_from_string("2024-07-11 13:00:03"));
    auto result_ask2 = entry.publish(ask2, 1);
    REQUIRE(result_ask2);
    REQUIRE(result_ask2->get_sequence() == Beam::Sequence(53));
  }

  TEST_CASE("publish_time_and_sale") {
    auto initial_sequences = SecurityEntry::InitialSequences();
    initial_sequences.m_next_time_and_sale_sequence = Beam::Sequence(100);
    auto security = Security("TST", TSX);
    auto entry = SecurityEntry(security, DEFAULT_VENUES,
      get_default_time_zone_database(), Money::ONE, initial_sequences);
    auto time_and_sale1 = TimeAndSale(time_from_string("2024-07-11 14:00:00"),
      Money::ONE, 100, TimeAndSale::Condition(), "TSX", "", "");
    auto result1 = entry.publish(time_and_sale1, 1);
    REQUIRE(result1);
    REQUIRE(***result1 == time_and_sale1);
    REQUIRE((*result1)->get_index() == security);
    REQUIRE(result1->get_sequence() == Beam::Sequence(100));
    auto time_and_sale2 = TimeAndSale(time_from_string("2024-07-11 14:00:01"),
      Money::ONE + Money::CENT, 200, TimeAndSale::Condition(), "TSX", "", "");
    auto result2 = entry.publish(time_and_sale2, 1);
    REQUIRE(result2);
    REQUIRE(***result2 == time_and_sale2);
    REQUIRE((*result2)->get_index() == security);
    REQUIRE(result2->get_sequence() == Beam::Sequence(101));
  }

  TEST_CASE("snapshot") {
    auto initial_sequences = SecurityEntry::InitialSequences();
    initial_sequences.m_next_bbo_quote_sequence = Beam::Sequence(10);
    initial_sequences.m_next_book_quote_sequence = Beam::Sequence(20);
    initial_sequences.m_next_time_and_sale_sequence = Beam::Sequence(30);
    auto security = Security("TST", TSX);
    auto entry = SecurityEntry(security, DEFAULT_VENUES,
      get_default_time_zone_database(), Money::ONE, initial_sequences);
    auto expected_snapshot = SecuritySnapshot(security);
    auto bbo1 =
      BboQuote(make_bid(10 * Money::CENT, 100), make_ask(11 * Money::CENT, 200),
        time_from_string("2024-07-11 13:00:00"));
    expected_snapshot.m_bbo_quote = entry.publish(bbo1, 1).get();
    auto book_bid1 =
      BookQuote("MP1", false, TSX, make_bid(10 * Money::CENT, 100),
        time_from_string("2024-07-11 13:00:00"));
    expected_snapshot.m_bids.push_back(entry.publish(book_bid1, 1).get());
    auto book_ask1 =
      BookQuote("MP1", false, TSX, make_ask(11 * Money::CENT, 200),
        time_from_string("2024-07-11 13:00:00"));
    expected_snapshot.m_asks.push_back(entry.publish(book_ask1, 1).get());
    auto ts1 = TimeAndSale(time_from_string("2024-07-11 13:00:00"),
      10 * Money::CENT, 100, TimeAndSale::Condition(), "TSX", "", "");
    expected_snapshot.m_time_and_sale = entry.publish(ts1, 1).get();
    REQUIRE((entry.load_snapshot() == expected_snapshot));
    REQUIRE(**entry.get_bbo_quote() == expected_snapshot.m_bbo_quote);
    auto bbo2 =
      BboQuote(make_bid(12 * Money::CENT, 300), make_ask(13 * Money::CENT, 400),
        time_from_string("2024-07-11 13:00:01"));
    expected_snapshot.m_bbo_quote = entry.publish(bbo2, 1).get();
    REQUIRE((entry.load_snapshot() == expected_snapshot));
    REQUIRE(**entry.get_bbo_quote() == expected_snapshot.m_bbo_quote);
    auto book_bid2 =
      BookQuote("MP2", false, TSX, make_bid(9 * Money::CENT, 500),
        time_from_string("2024-07-11 13:00:02"));
    expected_snapshot.m_bids.push_back(entry.publish(book_bid2, 1).get());
    auto book_ask2 =
      BookQuote("MP2", false, TSX, make_ask(14 * Money::CENT, 600),
        time_from_string("2024-07-11 13:00:02"));
    expected_snapshot.m_asks.push_back(entry.publish(book_ask2, 1).get());
    REQUIRE((entry.load_snapshot() == expected_snapshot));
    REQUIRE(**entry.get_bbo_quote() == expected_snapshot.m_bbo_quote);
    auto book_bid1_update =
      BookQuote("MP1", false, TSX, make_bid(10 * Money::CENT, 700),
        time_from_string("2024-07-11 13:00:03"));
    expected_snapshot.m_bids[0] = entry.publish(book_bid1_update, 1).get();
    auto book_ask1_update =
      BookQuote("MP1", false, TSX, make_ask(11 * Money::CENT, 800),
        time_from_string("2024-07-11 13:00:03"));
    expected_snapshot.m_asks[0] = entry.publish(book_ask1_update, 1).get();
    REQUIRE((entry.load_snapshot() == expected_snapshot));
    REQUIRE(**entry.get_bbo_quote() == expected_snapshot.m_bbo_quote);
    auto ts2 = TimeAndSale(time_from_string("2024-07-11 13:00:04"),
      12 * Money::CENT, 1000, TimeAndSale::Condition(), "TSX", "", "");
    expected_snapshot.m_time_and_sale = entry.publish(ts2, 1).get();
    REQUIRE((entry.load_snapshot() == expected_snapshot));
    REQUIRE(**entry.get_bbo_quote() == expected_snapshot.m_bbo_quote);
  }

  TEST_CASE("technicals") {
    auto initial_sequences = SecurityEntry::InitialSequences();
    auto security = Security("TST", TSX);
    auto entry = SecurityEntry(security, DEFAULT_VENUES,
      get_default_time_zone_database(), Money::ONE, initial_sequences);
    auto technicals1 = entry.get_security_technicals();
    REQUIRE(technicals1.m_close == Money::ONE);
    REQUIRE(technicals1.m_open == Money::ZERO);
    REQUIRE(technicals1.m_high == Money::ZERO);
    REQUIRE(technicals1.m_low == Money::ZERO);
    REQUIRE(technicals1.m_volume == 0);
    entry.publish(BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 100),
      time_from_string("2024-07-11 09:30:00")), 1);
    auto ts1 = TimeAndSale(time_from_string("2024-07-11 09:30:01"),
      Money::ONE + 5 * Money::CENT, 100, TimeAndSale::Condition(), "TSE", "",
      "");
    entry.publish(ts1, 1);
    auto technicals2 = entry.get_security_technicals();
    REQUIRE(technicals2.m_close == Money::ONE);
    REQUIRE(technicals2.m_open == Money::ONE + 5 * Money::CENT);
    REQUIRE(technicals2.m_high == Money::ONE + 5 * Money::CENT);
    REQUIRE(technicals2.m_low == Money::ONE + 5 * Money::CENT);
    REQUIRE(technicals2.m_volume == 100);
    auto ts2 = TimeAndSale(time_from_string("2024-07-11 09:30:02"),
      Money::ONE + 10 * Money::CENT, 200, TimeAndSale::Condition(), "TSE", "",
      "");
    entry.publish(ts2, 1);
    auto technicals3 = entry.get_security_technicals();
    REQUIRE(technicals3.m_high == Money::ONE + 10 * Money::CENT);
    REQUIRE(technicals3.m_volume == 300);
    auto ts3 = TimeAndSale(time_from_string("2024-07-11 09:30:03"),
      Money::ONE + 7 * Money::CENT, 50, TimeAndSale::Condition(), "TSE", "",
      "");
    entry.publish(ts3, 1);
    auto technicals4 = entry.get_security_technicals();
    REQUIRE(technicals4.m_high == Money::ONE + 10 * Money::CENT);
    REQUIRE(technicals4.m_low == Money::ONE + 5 * Money::CENT);
    REQUIRE(technicals4.m_volume == 350);
    auto ts4 = TimeAndSale(time_from_string("2024-07-11 09:30:04"),
      Money::ONE + 2 * Money::CENT, 150, TimeAndSale::Condition(), "TSE", "",
      "");
    entry.publish(ts4, 1);
    auto technicals5 = entry.get_security_technicals();
    REQUIRE(technicals5.m_low == Money::ONE + 2 * Money::CENT);
    REQUIRE(technicals5.m_volume == 500);
    entry.publish(BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 100),
      time_from_string("2024-07-12 09:30:00")), 1);
    auto technicals6 = entry.get_security_technicals();
    REQUIRE(technicals6.m_close == Money::ONE + 2 * Money::CENT);
    REQUIRE(technicals6.m_open == Money::ZERO);
    REQUIRE(technicals6.m_high == Money::ZERO);
    REQUIRE(technicals6.m_low == Money::ZERO);
    REQUIRE(technicals6.m_volume == 0);
  }

  TEST_CASE("load_initial_sequences") {
    auto data_store = LocalHistoricalDataStore();
    auto security1 = Security("TST", TSX);
    auto security2 = Security("S32", ASX);
    auto security3 = Security("RY", TSX);
    data_store.store(SequencedSecurityBboQuote(
      SecurityBboQuote(BboQuote(), security1), Beam::Sequence(5)));
    data_store.store(SequencedSecurityBboQuote(
      SecurityBboQuote(BboQuote(), security1), Beam::Sequence(3)));
    data_store.store(SequencedSecurityBboQuote(
      SecurityBboQuote(BboQuote(), security1), Beam::Sequence(7)));
    data_store.store(SequencedSecurityBookQuote(
      SecurityBookQuote(BookQuote(), security1), Beam::Sequence(12)));
    data_store.store(SequencedSecurityBookQuote(
      SecurityBookQuote(BookQuote(), security1), Beam::Sequence(18)));
    data_store.store(SequencedSecurityTimeAndSale(SecurityTimeAndSale(
      TimeAndSale(), security1), Beam::Sequence(22)));
    data_store.store(SequencedSecurityBboQuote(
      SecurityBboQuote(BboQuote(), security2), Beam::Sequence(1)));
    data_store.store(SequencedSecurityBookQuote(
      SecurityBookQuote(BookQuote(), security2), Beam::Sequence(2)));
    data_store.store(SequencedSecurityTimeAndSale(
      SecurityTimeAndSale(TimeAndSale(), security2), Beam::Sequence(3)));
    auto sequences1 = load_initial_sequences(data_store, security1);
    REQUIRE(sequences1.m_next_bbo_quote_sequence == Beam::Sequence(8));
    REQUIRE(sequences1.m_next_book_quote_sequence == Beam::Sequence(19));
    REQUIRE(sequences1.m_next_time_and_sale_sequence == Beam::Sequence(23));
    auto sequences2 = load_initial_sequences(data_store, security2);
    REQUIRE(sequences2.m_next_bbo_quote_sequence == Beam::Sequence(2));
    REQUIRE(sequences2.m_next_book_quote_sequence == Beam::Sequence(3));
    REQUIRE(sequences2.m_next_time_and_sale_sequence == Beam::Sequence(4));
    auto sequences3 = load_initial_sequences(data_store, security3);
    REQUIRE(sequences3.m_next_bbo_quote_sequence == Beam::Sequence::FIRST);
    REQUIRE(sequences3.m_next_book_quote_sequence == Beam::Sequence::FIRST);
    REQUIRE(sequences3.m_next_time_and_sale_sequence == Beam::Sequence::FIRST);
  }
}
