#include "Nexus/MarketDataServiceTests/MarketEntryTester.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/MarketEntry.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace std;

namespace {
  Security TEST_SECURITY("TST", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
}

void MarketEntryTester::TestPublishOrderImbalance() {
  MarketEntry::InitialSequences initialSequences;
  initialSequences.m_nextOrderImbalanceSequence = Beam::Queries::Sequence(5);
  MarketEntry entry(DefaultMarkets::NASDAQ(), initialSequences);
  OrderImbalance orderImbalanceA(TEST_SECURITY, Side::BID, 1000, Money::ONE,
    second_clock::universal_time());
  optional<SequencedMarketOrderImbalance> sequencedOrderImbalanceA =
    entry.PublishOrderImbalance(orderImbalanceA, 0);
  CPPUNIT_ASSERT(sequencedOrderImbalanceA.is_initialized());
  CPPUNIT_ASSERT(sequencedOrderImbalanceA->GetSequence() >=
    Beam::Queries::Sequence(5));
  CPPUNIT_ASSERT((*sequencedOrderImbalanceA)->GetIndex() ==
    DefaultMarkets::NASDAQ());
  CPPUNIT_ASSERT(orderImbalanceA == **sequencedOrderImbalanceA);
  OrderImbalance orderImbalanceB(TEST_SECURITY, Side::ASK, 2000, Money::CENT,
    second_clock::universal_time());
  optional<SequencedMarketOrderImbalance> sequencedOrderImbalanceB =
    entry.PublishOrderImbalance(orderImbalanceB, 0);
  CPPUNIT_ASSERT(sequencedOrderImbalanceB.is_initialized());
  CPPUNIT_ASSERT(sequencedOrderImbalanceB->GetSequence() >
    sequencedOrderImbalanceA->GetSequence());
  CPPUNIT_ASSERT((*sequencedOrderImbalanceB)->GetIndex() ==
    DefaultMarkets::NASDAQ());
  CPPUNIT_ASSERT(orderImbalanceB == **sequencedOrderImbalanceB);
}
