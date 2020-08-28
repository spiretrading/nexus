#ifndef NEXUS_MARKETDATASECURITYMARKETDATAQUERY_HPP
#define NEXUS_MARKETDATASECURITYMARKETDATAQUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/QueueWriter.hpp>
#include <Beam/Routines/Routine.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/MarketQuote.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/Definitions/Security.hpp"
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

  //! Defines the type of query used to receive a Security's market data.
  using SecurityMarketDataQuery = Beam::Queries::BasicQuery<Security>;

  //! Submits a query for real time BboQuotes with a snapshot.
  /*!
    \param security The Security to query.
    \param client The MarketDataClient to submit the query to.
    \param queue The Queue to write to.
  */
  template<typename MarketDataClient>
  void QueryRealTimeWithSnapshot(Security security, MarketDataClient& client,
      const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) {
    Beam::Routines::Spawn(
      [=, &client] {
        SecurityMarketDataQuery snapshotQuery;
        snapshotQuery.SetIndex(security);
        snapshotQuery.SetRange(Beam::Queries::Sequence::First(),
          Beam::Queries::Sequence::Present());
        snapshotQuery.SetSnapshotLimit(
          Beam::Queries::SnapshotLimit::Type::TAIL, 1);
        auto snapshotQueue = std::make_shared<Beam::Queue<SequencedBboQuote>>();
        client.QueryBboQuotes(snapshotQuery, snapshotQueue);
        SequencedBboQuote snapshot;
        try {
          snapshot = snapshotQueue->Pop();
          queue->Push(std::move(*snapshot));
        } catch(const Beam::PipeBrokenException&) {
          queue->Break();
          return;
        }
        SecurityMarketDataQuery continuationQuery;
        continuationQuery.SetIndex(std::move(security));
        continuationQuery.SetRange(
          Beam::Queries::Increment(snapshot.GetSequence()),
          Beam::Queries::Sequence::Last());
        continuationQuery.SetSnapshotLimit(
          Beam::Queries::SnapshotLimit::Unlimited());
        continuationQuery.SetInterruptionPolicy(
          Beam::Queries::InterruptionPolicy::IGNORE_CONTINUE);
        client.QueryBboQuotes(continuationQuery, queue);
      });
  }
}
}

#endif
