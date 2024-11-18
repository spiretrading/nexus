#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/SecurityEntry.hpp"

using namespace Beam;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;

namespace {
  const auto SECURITY_A = Security("TST", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  const auto TEST_SOURCE = 321;

  struct Fixture {
    IncrementalTimeClient m_timeClient;

    auto PublishBboQuote(SecurityEntry& entry, Money bidPrice,
        Quantity bidQuantity, Money askPrice, Quantity askQuantity,
        Beam::Queries::Sequence expectedSequence) {
      auto bboQuote = BboQuote(Quote(bidPrice, bidQuantity, Side::BID),
        Quote(askPrice, askQuantity, Side::ASK), m_timeClient.GetTime());
      auto sequencedBboQuote = entry.PublishBboQuote(bboQuote, TEST_SOURCE);
      auto timestampedSequence = EncodeTimestamp(bboQuote.m_timestamp,
        expectedSequence);
      REQUIRE(sequencedBboQuote.is_initialized());
      REQUIRE(sequencedBboQuote->GetSequence() == timestampedSequence);
      REQUIRE((*sequencedBboQuote)->GetIndex() == SECURITY_A);
      REQUIRE(bboQuote == **sequencedBboQuote);
      REQUIRE(bboQuote == *entry.GetBboQuote());
      auto snapshot = entry.LoadSnapshot();
      REQUIRE(snapshot.is_initialized());
      REQUIRE(bboQuote == snapshot->m_bboQuote);
      return *sequencedBboQuote;
    }

    auto PublishMarketQuote(SecurityEntry& entry, MarketCode market,
        Money bidPrice, Quantity bidQuantity, Money askPrice,
        Quantity askQuantity, Beam::Queries::Sequence expectedSequence) {
      auto quote = MarketQuote(market, Quote(bidPrice, bidQuantity, Side::BID),
        Quote(askPrice, askQuantity, Side::ASK), m_timeClient.GetTime());
      auto sequencedQuote = entry.PublishMarketQuote(quote, TEST_SOURCE);
      auto timestampedSequence = EncodeTimestamp(quote.m_timestamp,
        expectedSequence);
      REQUIRE(sequencedQuote.is_initialized());
      REQUIRE(sequencedQuote->GetSequence() == timestampedSequence);
      REQUIRE((*sequencedQuote)->GetIndex() == SECURITY_A);
      REQUIRE(quote == **sequencedQuote);
      return *sequencedQuote;
    }

    auto TestMarketQuoteSnapshot(const SecurityEntry& entry,
        const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
      auto snapshot = entry.LoadSnapshot();
      REQUIRE(snapshot.is_initialized());
      REQUIRE(snapshot->m_marketQuotes.size() == marketQuotes.size());
      for(auto& marketQuote : marketQuotes) {
        REQUIRE_NOTHROW(static_cast<void>(
          snapshot->m_marketQuotes.at((*marketQuote)->m_market)));
        REQUIRE(
          snapshot->m_marketQuotes.at((*marketQuote)->m_market).GetSequence() ==
          marketQuote.GetSequence());
        REQUIRE(
          snapshot->m_marketQuotes.at((*marketQuote)->m_market).GetValue() ==
          marketQuote.GetValue());
      }
    }

    auto PublishBookQuote(SecurityEntry& entry, std::string mpid,
        bool isPrimaryMpid, MarketCode market, Money price, Quantity quantity,
        Side side, Beam::Queries::Sequence expectedSequence,
        Quantity expectedQuantity) {
      auto quote = BookQuote(mpid, isPrimaryMpid, market,
        Quote(price, quantity, side), m_timeClient.GetTime());
      auto timestampedSequence = EncodeTimestamp(quote.m_timestamp,
        expectedSequence);
      auto sequencedQuote = entry.UpdateBookQuote(quote, TEST_SOURCE);
      REQUIRE(sequencedQuote.is_initialized());
      auto expectedQuote = quote;
      expectedQuote.m_quote.m_size = expectedQuantity;
      REQUIRE(sequencedQuote->GetSequence() == timestampedSequence);
      REQUIRE((*sequencedQuote)->GetIndex() == SECURITY_A);
      REQUIRE(expectedQuote == **sequencedQuote);
      return *sequencedQuote;
    }

    auto TestBookQuoteSnapshot(const SecurityEntry& entry,
        const std::vector<SequencedSecurityBookQuote>& expectedAsks,
        const std::vector<SequencedSecurityBookQuote>& expectedBids) {
      auto snapshot = entry.LoadSnapshot();
      REQUIRE(snapshot.is_initialized());
      REQUIRE(snapshot->m_askBook.size() == expectedAsks.size());
      REQUIRE(snapshot->m_bidBook.size() == expectedBids.size());
      for(auto& ask : expectedAsks) {
        auto sequenced_ask =
          Beam::Queries::SequencedValue(ask->GetValue(), ask.GetSequence());
        auto quoteIterator = find(snapshot->m_askBook.begin(),
          snapshot->m_askBook.end(), sequenced_ask);
        REQUIRE(quoteIterator != snapshot->m_askBook.end());
      }
      for(auto& bid : expectedBids) {
        auto sequenced_bid =
          Beam::Queries::SequencedValue(bid->GetValue(), bid.GetSequence());
        auto quoteIterator = find(snapshot->m_bidBook.begin(),
          snapshot->m_bidBook.end(), sequenced_bid);
        REQUIRE(quoteIterator != snapshot->m_bidBook.end());
      }
    }
  };
}

TEST_SUITE("SecurityEntry") {
  TEST_CASE_FIXTURE(Fixture, "publish_bbo_quote") {
    auto initialSequences = SecurityEntry::InitialSequences();
    auto entry = SecurityEntry(SECURITY_A, Money::ZERO, initialSequences);
    auto bboQuoteA = PublishBboQuote(entry, Money::ONE, 100,
      Money::ONE + Money::CENT, 100, Beam::Queries::Sequence(0));
    auto bboQuoteB = PublishBboQuote(entry, 2 * Money::ONE, 100,
      2 * Money::ONE + Money::CENT, 100, Beam::Queries::Sequence(1));
    auto bboQuoteC = PublishBboQuote(entry, 1 * Money::ONE, 100,
      1 * Money::ONE + Money::CENT, 100, Beam::Queries::Sequence(2));
  }

  TEST_CASE_FIXTURE(Fixture, "publish_market_quote") {
    auto initialSequences = SecurityEntry::InitialSequences();
    auto entry = SecurityEntry(SECURITY_A, Money::ZERO, initialSequences);
    auto nyseQuoteA = PublishMarketQuote(entry, DefaultMarkets::NYSE(),
      Money::ONE, 100, Money::ONE + Money::CENT, 100,
      Beam::Queries::Sequence(0));
    TestMarketQuoteSnapshot(entry, {nyseQuoteA});
    auto nasdaqQuoteA = PublishMarketQuote(entry, DefaultMarkets::NASDAQ(),
      Money::ONE, 100, Money::ONE + Money::CENT, 100,
      Beam::Queries::Sequence(1));
    TestMarketQuoteSnapshot(entry, {nyseQuoteA, nasdaqQuoteA});
    auto nyseQuoteB = PublishMarketQuote(entry, DefaultMarkets::NYSE(),
      2 * Money::ONE, 100, 2 * Money::ONE + Money::CENT, 100,
      Beam::Queries::Sequence(2));
    TestMarketQuoteSnapshot(entry, {nyseQuoteB, nasdaqQuoteA});
    auto nasdaqQuoteB = PublishMarketQuote(entry, DefaultMarkets::NASDAQ(),
      2 * Money::ONE, 100, 3 * Money::ONE + Money::CENT, 100,
      Beam::Queries::Sequence(3));
    TestMarketQuoteSnapshot(entry, {nyseQuoteB, nasdaqQuoteB});
  }

  TEST_CASE_FIXTURE(Fixture, "add_and_remove_book_quote") {
    auto initialSequences = SecurityEntry::InitialSequences();
    auto entry = SecurityEntry(SECURITY_A, Money::ZERO, initialSequences);
    auto abcBidA = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
      Money::ONE, 100, Side::BID, Beam::Queries::Sequence(0), 100);
    TestBookQuoteSnapshot(entry, {}, {abcBidA});
    auto abcAskA = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
      2 * Money::ONE, 100, Side::ASK, Beam::Queries::Sequence(1), 100);
    TestBookQuoteSnapshot(entry, {abcAskA}, {abcBidA});
    auto abcAskB = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
      2 * Money::ONE, 100, Side::ASK, Beam::Queries::Sequence(2), 200);
    TestBookQuoteSnapshot(entry, {abcAskB}, {abcBidA});
    auto abcAskC = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
      2 * Money::ONE, -200, Side::ASK, Beam::Queries::Sequence(3), 0);
    TestBookQuoteSnapshot(entry, {}, {abcBidA});
    auto abcBidB = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
      Money::ONE, -100, Side::BID, Beam::Queries::Sequence(4), 0);
    TestBookQuoteSnapshot(entry, {}, {});
    auto abcBidC = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
      Money::ONE, 100, Side::BID, Beam::Queries::Sequence(5), 100);
    TestBookQuoteSnapshot(entry, {}, {abcBidC});
    auto abcAskD = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
      2 * Money::ONE, 100, Side::ASK, Beam::Queries::Sequence(6), 100);
    TestBookQuoteSnapshot(entry, {abcAskD}, {abcBidC});
  }

  TEST_CASE_FIXTURE(Fixture, "technicals_reset") {
    auto entry = SecurityEntry(SECURITY_A, Money::CENT,
      SecurityEntry::InitialSequences());
    {
      auto bboQuote = BboQuote(
        Quote(Money::ONE, 100, Side::BID),
        Quote(Money::ONE + Money::CENT, 100, Side::ASK),
        time_from_string("2020-12-03 04:30:00"));
      entry.PublishBboQuote(bboQuote, TEST_SOURCE);
    }
    {
      auto timeAndSale = TimeAndSale(
        time_from_string("2020-12-03 4:31:00"), Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "XNAS", "CITI", "GS");
      entry.PublishTimeAndSale(timeAndSale, TEST_SOURCE);
      auto technicals = entry.GetSecurityTechnicals();
      REQUIRE(technicals.m_open == Money::ONE);
      REQUIRE(technicals.m_close == Money::CENT);
      REQUIRE(technicals.m_high == Money::ONE);
      REQUIRE(technicals.m_low == Money::ONE);
      REQUIRE(technicals.m_volume == 100);
    }
    {
      auto timeAndSale = TimeAndSale(
        time_from_string("2020-12-03 4:32:00"), Money::ONE + Money::CENT, 200,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "XNAS", "CITI", "GS");
      entry.PublishTimeAndSale(timeAndSale, TEST_SOURCE);
      auto technicals = entry.GetSecurityTechnicals();
      REQUIRE(technicals.m_open == Money::ONE);
      REQUIRE(technicals.m_close == Money::CENT);
      REQUIRE(technicals.m_high == Money::ONE + Money::CENT);
      REQUIRE(technicals.m_low == Money::ONE);
      REQUIRE(technicals.m_volume == 300);
    }
    {
      auto timeAndSale = TimeAndSale(
        time_from_string("2020-12-03 4:59:00"), 3 * Money::CENT, 600,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "XNAS", "CITI", "GS");
      entry.PublishTimeAndSale(timeAndSale, TEST_SOURCE);
      auto technicals = entry.GetSecurityTechnicals();
      REQUIRE(technicals.m_open == Money::ONE);
      REQUIRE(technicals.m_close == Money::CENT);
      REQUIRE(technicals.m_high == Money::ONE + Money::CENT);
      REQUIRE(technicals.m_low == 3 * Money::CENT);
      REQUIRE(technicals.m_volume == 900);
    }
    {
      auto bboQuote = BboQuote(
        Quote(Money::ONE, 100, Side::BID),
        Quote(Money::ONE + Money::CENT, 100, Side::ASK),
        time_from_string("2020-12-03 05:00:00"));
      entry.PublishBboQuote(bboQuote, TEST_SOURCE);
      auto technicals = entry.GetSecurityTechnicals();
      REQUIRE(technicals.m_open == Money::ZERO);
      REQUIRE(technicals.m_close == 3 * Money::CENT);
      REQUIRE(technicals.m_high == Money::ZERO);
      REQUIRE(technicals.m_low == Money::ZERO);
      REQUIRE(technicals.m_volume == 0);
    }
    {
      auto timeAndSale = TimeAndSale(
        time_from_string("2020-12-03 5:01:00"), 3 * Money::ONE, 300,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "XNAS", "CITI", "GS");
      entry.PublishTimeAndSale(timeAndSale, TEST_SOURCE);
      auto technicals = entry.GetSecurityTechnicals();
      REQUIRE(technicals.m_open == 3 * Money::ONE);
      REQUIRE(technicals.m_close == 3 * Money::CENT);
      REQUIRE(technicals.m_high == 3 * Money::ONE);
      REQUIRE(technicals.m_low == 3 * Money::ONE);
      REQUIRE(technicals.m_volume == 300);
    }
    {
      auto bboQuote = BboQuote(
        Quote(Money::ONE, 100, Side::BID),
        Quote(Money::ONE + Money::CENT, 100, Side::ASK),
        time_from_string("2020-12-10 05:00:00"));
      entry.PublishBboQuote(bboQuote, TEST_SOURCE);
      auto technicals = entry.GetSecurityTechnicals();
      REQUIRE(technicals.m_open == Money::ZERO);
      REQUIRE(technicals.m_close == 3 * Money::ONE);
      REQUIRE(technicals.m_high == Money::ZERO);
      REQUIRE(technicals.m_low == Money::ZERO);
      REQUIRE(technicals.m_volume == 0);
    }
    {
      auto timeAndSale = TimeAndSale(
        time_from_string("2020-12-10 5:01:00"), 7 * Money::CENT, 900,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "XNAS", "CITI", "GS");
      entry.PublishTimeAndSale(timeAndSale, TEST_SOURCE);
      auto technicals = entry.GetSecurityTechnicals();
      REQUIRE(technicals.m_open == 7 * Money::CENT);
      REQUIRE(technicals.m_close == 3 * Money::ONE);
      REQUIRE(technicals.m_high == 7 * Money::CENT);
      REQUIRE(technicals.m_low == 7 * Money::CENT);
      REQUIRE(technicals.m_volume == 900);
    }
    {
      auto bboQuote = BboQuote(
        Quote(Money::ONE + Money::CENT, 100, Side::BID),
        Quote(Money::ONE + 2 * Money::CENT, 100, Side::ASK),
        time_from_string("2020-12-10 05:02:00"));
      entry.PublishBboQuote(bboQuote, TEST_SOURCE);
      auto technicals = entry.GetSecurityTechnicals();
      REQUIRE(technicals.m_open == 7 * Money::CENT);
      REQUIRE(technicals.m_close == 3 * Money::ONE);
      REQUIRE(technicals.m_high == 7 * Money::CENT);
      REQUIRE(technicals.m_low == 7 * Money::CENT);
      REQUIRE(technicals.m_volume == 900);
    }
  }
}
