#ifndef NEXUS_SECURITYENTRYTESTER_HPP
#define NEXUS_SECURITYENTRYTESTER_HPP
#include <optional>
#include <Beam/Queries/Queries.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTests.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Tests {

  /*  \class SecurityEntryTester
      \brief Tests the SecurityEntry class.
   */
  class SecurityEntryTester : public CPPUNIT_NS::TestFixture {
    public:
      virtual void setUp();

      virtual void tearDown();

      //! Tests publishing BboQuotes.
      void TestPublishBboQuote();

      //! Tests publishing MarketQuotes.
      void TestPublishMarketQuote();

      //! Tests adding and then removing BookQuotes.
      void TestAddAndRemoveBookQuote();

    private:
      std::optional<Beam::TimeService::IncrementalTimeClient> m_timeClient;

      SequencedSecurityBboQuote PublishBboQuote(SecurityEntry& entry,
        Money bidPrice, Quantity bidQuantity, Money askPrice,
        Quantity askQuantity, const Beam::Queries::Sequence& expectedSequence);
      SequencedSecurityMarketQuote PublishMarketQuote(SecurityEntry& entry,
        MarketCode market, Money bidPrice, Quantity bidQuantity, Money askPrice,
        Quantity askQuantity, const Beam::Queries::Sequence& expectedSequence);
      void TestMarketQuoteSnapshot(const SecurityEntry& entry,
        const std::vector<SequencedSecurityMarketQuote>& marketQuotes);
      SequencedSecurityBookQuote PublishBookQuote(SecurityEntry& entry,
        std::string mpid, bool isPrimaryMpid, MarketCode market, Money price,
        Quantity quantity, Side side,
        const Beam::Queries::Sequence& expectedSequence,
        Quantity expectedQuantity);
      void TestBookQuoteSnapshot(const SecurityEntry& entry,
        const std::vector<SequencedSecurityBookQuote>& expectedAsks,
        const std::vector<SequencedSecurityBookQuote>& expectedBids);

      CPPUNIT_TEST_SUITE(SecurityEntryTester);
        CPPUNIT_TEST(TestPublishBboQuote);
        CPPUNIT_TEST(TestPublishMarketQuote);
        CPPUNIT_TEST(TestAddAndRemoveBookQuote);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
