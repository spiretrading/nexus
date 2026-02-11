#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/TickerSnapshot.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("TickerSnapshot") {
  TEST_CASE("shuttle") {
    auto snapshot = TickerSnapshot();
    snapshot.m_ticker = parse_ticker("BCA.TSX");
    snapshot.m_bbo_quote = SequencedBboQuote(
      BboQuote(make_bid(50 * Money::CENT, 213), make_ask(55 * Money::CENT, 312),
        time_from_string("2021-02-25 15:30:05.000")), Beam::Sequence(20000));
    snapshot.m_time_and_sale = SequencedTimeAndSale(
      TimeAndSale(time_from_string("2021-01-12 15:30:05.000"), Money(42), 123,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::CLOSE, "X"), "CTR",
        "B42", "S12"), Beam::Sequence(30000));
    test_round_trip_shuttle(snapshot);
    snapshot.m_asks.push_back(SequencedBookQuote(
      BookQuote("MP", false, CHIC, make_ask(12 * Money::CENT, 222),
        time_from_string("2021-01-11 15:30:05.000")), Beam::Sequence(12345)));
    snapshot.m_asks.push_back(SequencedBookQuote(
      BookQuote("MP1", true, OMGA, make_ask(13 * Money::CENT, 2222),
        time_from_string("2021-01-11 15:30:06.000")), Beam::Sequence(12346)));
    snapshot.m_bids.push_back(SequencedBookQuote(
      BookQuote("MP", false, PURE, make_bid(9 * Money::CENT, 44),
        time_from_string("2021-01-11 15:30:05.000")), Beam::Sequence(12347)));
    snapshot.m_bids.push_back(SequencedBookQuote(
      BookQuote("MP", false, PURE, make_bid(8 * Money::CENT, 42),
        time_from_string("2021-01-11 15:30:05.000")), Beam::Sequence(12348)));
    test_round_trip_shuttle(snapshot);
  }
}
