#include "Nexus/MarketDataServiceTests/SecurityEntryTester.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/SecurityEntry.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace std;

namespace {
  Security TEST_SECURITY("TST", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  int TEST_SOURCE = 321;
}

void SecurityEntryTester::setUp() {
  m_timeClient.Initialize();
}

void SecurityEntryTester::tearDown() {
  m_timeClient.Reset();
}

void SecurityEntryTester::TestPublishBboQuote() {
  SecurityEntry::InitialSequences initialSequences;
  initialSequences.m_nextBboQuoteSequence = Beam::Queries::Sequence(5);
  SecurityEntry entry(TEST_SECURITY, Money::ZERO, initialSequences);
  SequencedSecurityBboQuote bboQuoteA = PublishBboQuote(entry, Money::ONE, 100,
    Money::ONE + Money::CENT, 100,
    Decrement(initialSequences.m_nextBboQuoteSequence));
  SequencedSecurityBboQuote bboQuoteB = PublishBboQuote(entry, 2 * Money::ONE,
    100, 2 * Money::ONE + Money::CENT, 100, bboQuoteA.GetSequence());
  SequencedSecurityBboQuote bboQuoteC = PublishBboQuote(entry, 1 * Money::ONE,
    100, 1 * Money::ONE + Money::CENT, 100, bboQuoteB.GetSequence());
}

void SecurityEntryTester::TestPublishMarketQuote() {
  SecurityEntry::InitialSequences initialSequences;
  initialSequences.m_nextMarketQuoteSequence = Beam::Queries::Sequence(5);
  SecurityEntry entry(TEST_SECURITY, Money::ZERO, initialSequences);
  SequencedSecurityMarketQuote nyseQuoteA = PublishMarketQuote(entry,
    DefaultMarkets::NYSE(), Money::ONE, 100, Money::ONE + Money::CENT, 100,
    Decrement(initialSequences.m_nextMarketQuoteSequence));
  TestMarketQuoteSnapshot(entry, {nyseQuoteA});
  SequencedSecurityMarketQuote nasdaqQuoteA = PublishMarketQuote(entry,
    DefaultMarkets::NASDAQ(), Money::ONE, 100, Money::ONE + Money::CENT, 100,
    nyseQuoteA.GetSequence());
  TestMarketQuoteSnapshot(entry, {nyseQuoteA, nasdaqQuoteA});
  SequencedSecurityMarketQuote nyseQuoteB = PublishMarketQuote(entry,
    DefaultMarkets::NYSE(), 2 * Money::ONE, 100, 2 * Money::ONE + Money::CENT,
    100, nasdaqQuoteA.GetSequence());
  TestMarketQuoteSnapshot(entry, {nyseQuoteB, nasdaqQuoteA});
  SequencedSecurityMarketQuote nasdaqQuoteB = PublishMarketQuote(entry,
    DefaultMarkets::NASDAQ(), 2 * Money::ONE, 100, 3 * Money::ONE + Money::CENT,
    100, nyseQuoteB.GetSequence());
  TestMarketQuoteSnapshot(entry, {nyseQuoteB, nasdaqQuoteB});
}

void SecurityEntryTester::TestAddAndRemoveBookQuote() {
  SecurityEntry::InitialSequences initialSequences;
  initialSequences.m_nextBookQuoteSequence = Beam::Queries::Sequence(5);
  SecurityEntry entry(TEST_SECURITY, Money::ZERO, initialSequences);
  SequencedSecurityBookQuote abcBidA = PublishBookQuote(entry, "ABC", false,
    DefaultMarkets::NYSE(), Money::ONE, 100, Side::BID,
    Decrement(initialSequences.m_nextBookQuoteSequence), 100);
  TestBookQuoteSnapshot(entry, {}, {abcBidA});
  SequencedSecurityBookQuote abcAskA = PublishBookQuote(entry, "ABC", false,
    DefaultMarkets::NYSE(), 2 * Money::ONE, 100, Side::ASK,
    abcBidA.GetSequence(), 100);
  TestBookQuoteSnapshot(entry, {abcAskA}, {abcBidA});
  SequencedSecurityBookQuote abcAskB = PublishBookQuote(entry, "ABC", false,
    DefaultMarkets::NYSE(), 2 * Money::ONE, 100, Side::ASK,
    abcAskA.GetSequence(), 200);
  TestBookQuoteSnapshot(entry, {abcAskB}, {abcBidA});
  SequencedSecurityBookQuote abcAskC = PublishBookQuote(entry, "ABC", false,
    DefaultMarkets::NYSE(), 2 * Money::ONE, -200, Side::ASK,
    abcAskB.GetSequence(), 0);
  TestBookQuoteSnapshot(entry, {}, {abcBidA});
  SequencedSecurityBookQuote abcBidB = PublishBookQuote(entry, "ABC", false,
    DefaultMarkets::NYSE(), Money::ONE, -100, Side::BID, abcAskC.GetSequence(),
    0);
  TestBookQuoteSnapshot(entry, {}, {});
  SequencedSecurityBookQuote abcBidC = PublishBookQuote(entry, "ABC", false,
    DefaultMarkets::NYSE(), Money::ONE, 100, Side::BID, abcBidB.GetSequence(),
    100);
  TestBookQuoteSnapshot(entry, {}, {abcBidC});
  SequencedSecurityBookQuote abcAskD = PublishBookQuote(entry, "ABC", false,
    DefaultMarkets::NYSE(), 2 * Money::ONE, 100, Side::ASK,
    abcBidC.GetSequence(), 100);
  TestBookQuoteSnapshot(entry, {abcAskD}, {abcBidC});
}

SequencedSecurityBboQuote SecurityEntryTester::PublishBboQuote(
    SecurityEntry& entry, Money bidPrice, Quantity bidQuantity, Money askPrice,
    Quantity askQuantity, const Beam::Queries::Sequence& expectedSequence) {
  BboQuote bboQuote(Quote(bidPrice, bidQuantity, Side::BID),
    Quote(askPrice, askQuantity, Side::ASK), m_timeClient->GetTime());
  optional<SequencedSecurityBboQuote> sequencedBboQuote =
    entry.PublishBboQuote(bboQuote, TEST_SOURCE);
  CPPUNIT_ASSERT(sequencedBboQuote.is_initialized());
  CPPUNIT_ASSERT(sequencedBboQuote->GetSequence() > expectedSequence);
  CPPUNIT_ASSERT((*sequencedBboQuote)->GetIndex() == TEST_SECURITY);
  CPPUNIT_ASSERT(bboQuote == **sequencedBboQuote);
  CPPUNIT_ASSERT(bboQuote == *entry.GetBboQuote());
  optional<SecuritySnapshot> snapshot = entry.LoadSnapshot();
  CPPUNIT_ASSERT(snapshot.is_initialized());
  CPPUNIT_ASSERT(bboQuote == snapshot->m_bboQuote);
  return *sequencedBboQuote;
}

SequencedSecurityMarketQuote SecurityEntryTester::PublishMarketQuote(
    SecurityEntry& entry, MarketCode market, Money bidPrice,
    Quantity bidQuantity, Money askPrice, Quantity askQuantity,
    const Beam::Queries::Sequence& expectedSequence) {
  MarketQuote quote(market, Quote(bidPrice, bidQuantity, Side::BID),
    Quote(askPrice, askQuantity, Side::ASK), m_timeClient->GetTime());
  optional<SequencedSecurityMarketQuote> sequencedQuote =
    entry.PublishMarketQuote(quote, TEST_SOURCE);
  CPPUNIT_ASSERT(sequencedQuote.is_initialized());
  CPPUNIT_ASSERT(sequencedQuote->GetSequence() > expectedSequence);
  CPPUNIT_ASSERT((*sequencedQuote)->GetIndex() == TEST_SECURITY);
  CPPUNIT_ASSERT(quote == **sequencedQuote);
  return *sequencedQuote;
}

void SecurityEntryTester::TestMarketQuoteSnapshot(const SecurityEntry& entry,
    const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
  optional<SecuritySnapshot> snapshot = entry.LoadSnapshot();
  CPPUNIT_ASSERT(snapshot.is_initialized());
  CPPUNIT_ASSERT(snapshot->m_marketQuotes.size() == marketQuotes.size());
  for(const SequencedSecurityMarketQuote& marketQuote : marketQuotes) {
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
  BookQuote quote(mpid, isPrimaryMpid, market, Quote(price, quantity, side),
    m_timeClient->GetTime());
  optional<SequencedSecurityBookQuote> sequencedQuote =
    entry.UpdateBookQuote(quote, TEST_SOURCE);
  CPPUNIT_ASSERT(sequencedQuote.is_initialized());
  BookQuote expectedQuote = quote;
  expectedQuote.m_quote.m_size = expectedQuantity;
  CPPUNIT_ASSERT(sequencedQuote->GetSequence() > expectedSequence);
  CPPUNIT_ASSERT((*sequencedQuote)->GetIndex() == TEST_SECURITY);
  CPPUNIT_ASSERT(expectedQuote == **sequencedQuote);
  return *sequencedQuote;
}

void SecurityEntryTester::TestBookQuoteSnapshot(const SecurityEntry& entry,
    const vector<SequencedSecurityBookQuote>& expectedAsks,
    const vector<SequencedSecurityBookQuote>& expectedBids) {
  optional<SecuritySnapshot> snapshot = entry.LoadSnapshot();
  CPPUNIT_ASSERT(snapshot.is_initialized());
  CPPUNIT_ASSERT(snapshot->m_askBook.size() == expectedAsks.size());
  CPPUNIT_ASSERT(snapshot->m_bidBook.size() == expectedBids.size());
  for(const SequencedSecurityBookQuote& ask : expectedAsks) {
    auto quoteIterator = find(snapshot->m_askBook.begin(),
      snapshot->m_askBook.end(), ask);
    CPPUNIT_ASSERT(quoteIterator != snapshot->m_askBook.end());
  }
  for(const SequencedSecurityBookQuote& bid : expectedBids) {
    auto quoteIterator = find(snapshot->m_bidBook.begin(),
      snapshot->m_bidBook.end(), bid);
    CPPUNIT_ASSERT(quoteIterator != snapshot->m_bidBook.end());
  }
}
