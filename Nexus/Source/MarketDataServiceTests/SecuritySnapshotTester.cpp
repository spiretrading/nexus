#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("SecuritySnapshot") {
  TEST_CASE("shuttle") {
    auto snapshot = SecuritySnapshot();
    snapshot.m_security = Security("BCA", TSX);
    snapshot.m_bbo_quote =
      SequencedBboQuote(BboQuote(Quote(50 * Money::CENT, 213, Side::BID),
        Quote(55 * Money::CENT, 312, Side::ASK),
        time_from_string("2021-02-25 15:30:05.000")),
        Beam::Queries::Sequence(20000));
    snapshot.m_time_and_sale = SequencedTimeAndSale(
      TimeAndSale(time_from_string("2021-01-12 15:30:05.000"), Money(42), 123,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::CLOSE, "X"), "CTR",
        "B42", "S12"), Beam::Queries::Sequence(30000));
    Beam::Serialization::Tests::TestRoundTripShuttle(snapshot);
    snapshot.m_asks.push_back(SequencedBookQuote(
      BookQuote("MP", false, CHIC, Quote(12 * Money::CENT, 222, Side::ASK),
        time_from_string("2021-01-11 15:30:05.000")),
      Beam::Queries::Sequence(12345)));
    snapshot.m_asks.push_back(SequencedBookQuote(
      BookQuote("MP1", true, OMGA, Quote(13 * Money::CENT, 2222, Side::ASK),
        time_from_string("2021-01-11 15:30:06.000")),
      Beam::Queries::Sequence(12346)));
    snapshot.m_bids.push_back(SequencedBookQuote(
      BookQuote("MP", false, PURE, Quote(9 * Money::CENT, 44, Side::BID),
        time_from_string("2021-01-11 15:30:05.000")),
      Beam::Queries::Sequence(12347)));
    snapshot.m_bids.push_back(SequencedBookQuote(
      BookQuote("MP", false, PURE, Quote(8 * Money::CENT, 42, Side::BID),
        time_from_string("2021-01-11 15:30:05.000")),
      Beam::Queries::Sequence(12348)));
    Beam::Serialization::Tests::TestRoundTripShuttle(snapshot);
  }
}
