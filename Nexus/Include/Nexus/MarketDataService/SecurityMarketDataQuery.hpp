#ifndef NEXUS_MARKET_DATA_SECURITY_MARKET_DATA_QUERY_HPP
#define NEXUS_MARKET_DATA_SECURITY_MARKET_DATA_QUERY_HPP
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/PagedQuery.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/Routines/Routine.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/MarketQuote.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus {
  using SequencedBboQuote = Beam::Queries::SequencedValue<BboQuote>;
  using SequencedBookQuote = Beam::Queries::SequencedValue<BookQuote>;
  using SequencedMarketQuote = Beam::Queries::SequencedValue<MarketQuote>;
  using SequencedTimeAndSale = Beam::Queries::SequencedValue<TimeAndSale>;
  using SecurityBboQuote = Beam::Queries::IndexedValue<BboQuote, Security>;
  using SecurityBookQuote = Beam::Queries::IndexedValue<BookQuote, Security>;
  using SecurityMarketQuote =
    Beam::Queries::IndexedValue<MarketQuote, Security>;
  using SecurityTimeAndSale =
    Beam::Queries::IndexedValue<TimeAndSale, Security>;
  using SequencedSecurityBboQuote =
    Beam::Queries::SequencedValue<SecurityBboQuote>;
  using SequencedSecurityBookQuote =
    Beam::Queries::SequencedValue<SecurityBookQuote>;
  using SequencedSecurityMarketQuote =
    Beam::Queries::SequencedValue<SecurityMarketQuote>;
  using SequencedSecurityTimeAndSale =
    Beam::Queries::SequencedValue<SecurityTimeAndSale>;

namespace MarketDataService {

  /** Defines the type of query used to receive a Security's market data. */
  using SecurityMarketDataQuery = Beam::Queries::BasicQuery<Security>;

  /** Defines the type of query used to load SecurityInfo objects. */
  using SecurityInfoQuery = Beam::Queries::PagedQuery<Region, Security>;

  /**
   * Submits a query for real time BboQuotes with a snapshot.
   * @param security The Security to query.
   * @param client The MarketDataClient to submit the query to.
   * @param queue The Queue to write to.
   */
  template<typename MarketDataClient>
  Beam::Routines::Routine::Id QueryRealTimeWithSnapshot(Security security,
      MarketDataClient&& client, Beam::ScopedQueueWriter<BboQuote> queue) {
    return Beam::Routines::Spawn(
      [=, security = std::move(security),
          client = Beam::CapturePtr<MarketDataClient>(client),
          queue = std::move(queue)] () mutable {
        auto snapshotQueue = std::make_shared<Beam::Queue<SequencedBboQuote>>();
        client->QueryBboQuotes(Beam::Queries::BuildLatestQuery(security),
          snapshotQueue);
        auto snapshot = SequencedBboQuote();
        try {
          snapshot = snapshotQueue->Pop();
          queue.Push(std::move(*snapshot));
        } catch(const Beam::PipeBrokenException&) {
          return;
        }
        auto continuationQuery = SecurityMarketDataQuery();
        continuationQuery.SetIndex(std::move(security));
        continuationQuery.SetRange(
          Beam::Queries::Increment(snapshot.GetSequence()),
          Beam::Queries::Sequence::Last());
        continuationQuery.SetSnapshotLimit(
          Beam::Queries::SnapshotLimit::Unlimited());
        continuationQuery.SetInterruptionPolicy(
          Beam::Queries::InterruptionPolicy::IGNORE_CONTINUE);
        client->QueryBboQuotes(continuationQuery, std::move(queue));
      });
  }

  /**
   * Returns a query to retrieve the SecurityInfo for a single security.
   * @param security The Security to query.
   * @return The SecurityInfo for the given
   */
  inline SecurityInfoQuery MakeSecurityInfoQuery(const Security& security) {
    auto query = SecurityInfoQuery();
    query.SetIndex(security);
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::FromHead(1));
    return query;
  }

  /**
   * Submits a query to retrieve the SecurityInfo for a single security.
   * @param security The Security to query.
   * @param client The MarketDataClient to submit the query to.
   * @return The SecurityInfo for the given <i>security</i>.
   */
  template<typename MarketDataClient>
  boost::optional<SecurityInfo> LoadSecurityInfo(const Security& security,
      MarketDataClient& client) {
    auto result = client.QuerySecurityInfo(MakeSecurityInfoQuery(security));
    if(!result.empty()) {
      return result.front();
    }
    return boost::none;
  }
}
}

#endif
