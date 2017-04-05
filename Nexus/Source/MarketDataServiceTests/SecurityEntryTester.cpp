#include "Nexus/MarketDataServiceTests/SecurityEntryTester.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/SecurityEntry.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace std;

namespace {
  Security TEST_SECURITY{"TST", DefaultMarkets::NASDAQ(),
    DefaultCountries::US()};
  auto TEST_SOURCE = 321;
}

void SecurityEntryTester::setUp() {
  m_timeClient.Initialize();
}

void SecurityEntryTester::tearDown() {
  m_timeClient.Reset();
}

void SecurityEntryTester::TestPublishBboQuote() {
  SecurityEntry::InitialSequences initialSequences;
  SecurityEntry entry{TEST_SECURITY, Money::ZERO, initialSequences};
  auto bboQuoteA = PublishBboQuote(entry, Money::ONE, 100,
    Money::ONE + Money::CENT, 100, Queries::Sequence{0});
  auto bboQuoteB = PublishBboQuote(entry, 2 * Money::ONE, 100,
    2 * Money::ONE + Money::CENT, 100, Queries::Sequence{1});
  auto bboQuoteC = PublishBboQuote(entry, 1 * Money::ONE, 100,
    1 * Money::ONE + Money::CENT, 100, Queries::Sequence{2});
}

void SecurityEntryTester::TestPublishMarketQuote() {
  SecurityEntry::InitialSequences initialSequences;
  SecurityEntry entry{TEST_SECURITY, Money::ZERO, initialSequences};
  auto nyseQuoteA = PublishMarketQuote(entry, DefaultMarkets::NYSE(),
    Money::ONE, 100, Money::ONE + Money::CENT, 100, Queries::Sequence{0});
  TestMarketQuoteSnapshot(entry, {nyseQuoteA});
  auto nasdaqQuoteA = PublishMarketQuote(entry, DefaultMarkets::NASDAQ(),
    Money::ONE, 100, Money::ONE + Money::CENT, 100, Queries::Sequence{1});
  TestMarketQuoteSnapshot(entry, {nyseQuoteA, nasdaqQuoteA});
  auto nyseQuoteB = PublishMarketQuote(entry, DefaultMarkets::NYSE(),
    2 * Money::ONE, 100, 2 * Money::ONE + Money::CENT, 100,
    Queries::Sequence{2});
  TestMarketQuoteSnapshot(entry, {nyseQuoteB, nasdaqQuoteA});
  auto nasdaqQuoteB = PublishMarketQuote(entry, DefaultMarkets::NASDAQ(),
    2 * Money::ONE, 100, 3 * Money::ONE + Money::CENT, 100,
    Queries::Sequence{3});
  TestMarketQuoteSnapshot(entry, {nyseQuoteB, nasdaqQuoteB});
}

void SecurityEntryTester::TestAddAndRemoveBookQuote() {
  SecurityEntry::InitialSequences initialSequences;
  SecurityEntry entry{TEST_SECURITY, Money::ZERO, initialSequences};
  auto abcBidA = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
    Money::ONE, 100, Side::BID, Queries::Sequence{0}, 100);
  TestBookQuoteSnapshot(entry, {}, {abcBidA});
  auto abcAskA = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
    2 * Money::ONE, 100, Side::ASK, Queries::Sequence{1}, 100);
  TestBookQuoteSnapshot(entry, {abcAskA}, {abcBidA});
  auto abcAskB = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
    2 * Money::ONE, 100, Side::ASK, Queries::Sequence{2}, 200);
  TestBookQuoteSnapshot(entry, {abcAskB}, {abcBidA});
  auto abcAskC = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
    2 * Money::ONE, -200, Side::ASK, Queries::Sequence{3}, 0);
  TestBookQuoteSnapshot(entry, {}, {abcBidA});
  auto abcBidB = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
    Money::ONE, -100, Side::BID, Queries::Sequence{4}, 0);
  TestBookQuoteSnapshot(entry, {}, {});
  auto abcBidC = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
    Money::ONE, 100, Side::BID, Queries::Sequence{5}, 100);
  TestBookQuoteSnapshot(entry, {}, {abcBidC});
  auto abcAskD = PublishBookQuote(entry, "ABC", false, DefaultMarkets::NYSE(),
    2 * Money::ONE, 100, Side::ASK, Queries::Sequence{6}, 100);
  TestBookQuoteSnapshot(entry, {abcAskD}, {abcBidC});
}

SequencedSecurityBboQuote SecurityEntryTester::PublishBboQuote(
    SecurityEntry& entry, Money bidPrice, Quantity bidQuantity, Money askPrice,
    Quantity askQuantity, const Beam::Queries::Sequence& expectedSequence) {
  BboQuote bboQuote{Quote{bidPrice, bidQuantity, Side::BID},
    Quote{askPrice, askQuantity, Side::ASK}, m_timeClient->GetTime()};
  auto sequencedBboQuote = entry.PublishBboQuote(bboQuote, TEST_SOURCE);
  auto timestampedSequence = EncodeTimestamp(bboQuote.m_timestamp,
    expectedSequence);
  CPPUNIT_ASSERT(sequencedBboQuote.is_initialized());
  CPPUNIT_ASSERT(sequencedBboQuote->GetSequence() == timestampedSequence);
  CPPUNIT_ASSERT((*sequencedBboQuote)->GetIndex() == TEST_SECURITY);
  CPPUNIT_ASSERT(bboQuote == **sequencedBboQuote);
  CPPUNIT_ASSERT(bboQuote == *entry.GetBboQuote());
  auto snapshot = entry.LoadSnapshot();
  CPPUNIT_ASSERT(snapshot.is_initialized());
  CPPUNIT_ASSERT(bboQuote == snapshot->m_bboQuote);
  return *sequencedBboQuote;
}

SequencedSecurityMarketQuote SecurityEntryTester::PublishMarketQuote(
    SecurityEntry& entry, MarketCode market, Money bidPrice,
    Quantity bidQuantity, Money askPrice, Quantity askQuantity,
    const Beam::Queries::Sequence& expectedSequence) {
  MarketQuote quote{market, Quote{bidPrice, bidQuantity, Side::BID},
    Quote{askPrice, askQuantity, Side::ASK}, m_timeClient->GetTime()};
  auto sequencedQuote = entry.PublishMarketQuote(quote, TEST_SOURCE);
  auto timestampedSequence = EncodeTimestamp(quote.m_timestamp,
    expectedSequence);
  CPPUNIT_ASSERT(sequencedQuote.is_initialized());
  CPPUNIT_ASSERT(sequencedQuote->GetSequence() == timestampedSequence);
  CPPUNIT_ASSERT((*sequencedQuote)->GetIndex() == TEST_SECURITY);
  CPPUNIT_ASSERT(quote == **sequencedQuote);
  return *sequencedQuote;
}

void SecurityEntryTester::TestMarketQuoteSnapshot(const SecurityEntry& entry,
    const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
  auto snapshot = entry.LoadSnapshot();
  CPPUNIT_ASSERT(snapshot.is_initialized());
  CPPUNIT_ASSERT(snapshot->m_marketQuotes.size() == marketQuotes.size());
  for(auto& marketQuote : marketQuotes) {
    CPPUNIT_ASSERT_NO_THROW(snapshot->m_marketQuotes.at(
      (*marketQuote)->m_market));
    CPPUNIT_ASSERT(snapshot->m_marketQuotes.at((*marketQuote)->m_market) ==
      marketQuote);
  }
}

SequencedSecurityBookQuote SecurityEntryTester::PublishBookQuote(
    SecurityEntry& entry, string mpid, bool isPrimaryMpid, MarketCode market,
    Money price, Quantity quantity, Side side,
    const Beam::Queries::Sequence& expectedSequence,
    Quantity expectedQuantity) {
  BookQuote quote{mpid, isPrimaryMpid, market, Quote{price, quantity, side},
    m_timeClient->GetTime()};
  auto timestampedSequence = EncodeTimestamp(quote.m_timestamp,
    expectedSequence);
  auto sequencedQuote = entry.UpdateBookQuote(quote, TEST_SOURCE);
  CPPUNIT_ASSERT(sequencedQuote.is_initialized());
  auto expectedQuote = quote;
  expectedQuote.m_quote.m_size = expectedQuantity;
  CPPUNIT_ASSERT(sequencedQuote->GetSequence() == timestampedSequence);
  CPPUNIT_ASSERT((*sequencedQuote)->GetIndex() == TEST_SECURITY);
  CPPUNIT_ASSERT(expectedQuote == **sequencedQuote);
  return *sequencedQuote;
}

void SecurityEntryTester::TestBookQuoteSnapshot(const SecurityEntry& entry,
    const vector<SequencedSecurityBookQuote>& expectedAsks,
    const vector<SequencedSecurityBookQuote>& expectedBids) {
  auto snapshot = entry.LoadSnapshot();
  CPPUNIT_ASSERT(snapshot.is_initialized());
  CPPUNIT_ASSERT(snapshot->m_askBook.size() == expectedAsks.size());
  CPPUNIT_ASSERT(snapshot->m_bidBook.size() == expectedBids.size());
  for(auto& ask : expectedAsks) {
    auto quoteIterator = find(snapshot->m_askBook.begin(),
      snapshot->m_askBook.end(), ask);
    CPPUNIT_ASSERT(quoteIterator != snapshot->m_askBook.end());
  }
  for(auto& bid : expectedBids) {
    auto quoteIterator = find(snapshot->m_bidBook.begin(),
      snapshot->m_bidBook.end(), bid);
    CPPUNIT_ASSERT(quoteIterator != snapshot->m_bidBook.end());
  }
}
