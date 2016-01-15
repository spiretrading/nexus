#ifndef NEXUS_MARKETDATALOCALHISTORICALDATASTORETESTER_HPP
#define NEXUS_MARKETDATALOCALHISTORICALDATASTORETESTER_HPP
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTests.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Tests {

  /*  \class LocalHistoricalDataStoreTester
      \brief Tests the LocalHistoricalDataStore class.
   */
  class LocalHistoricalDataStoreTester : public CPPUNIT_NS::TestFixture {
    public:
      virtual void setUp();

      virtual void tearDown();

      //! Tests storing and loading a BboQuote.
      void TestStoreAndLoadBboQuote();

    private:
      Beam::DelayPtr<Beam::TimeService::IncrementalTimeClient> m_timeClient;

      SequencedSecurityBboQuote StoreBboQuote(
        LocalHistoricalDataStore& dataStore, Money bidPrice,
        Quantity bidQuantity, Money askPrice, Quantity askQuantity,
        const boost::posix_time::ptime& timestamp,
        const Beam::Queries::Sequence& sequence);

      void TestBboQuoteQuery(LocalHistoricalDataStore& dataStore,
        const Beam::Queries::Range& range,
        const Beam::Queries::SnapshotLimit& limit,
        const std::vector<SequencedSecurityBboQuote>& expectedResult);

      CPPUNIT_TEST_SUITE(LocalHistoricalDataStoreTester);
        CPPUNIT_TEST(TestStoreAndLoadBboQuote);
      CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
