#ifndef NEXUS_MARKETDATASECURITYMARKETDATAQUERY_HPP
#define NEXUS_MARKETDATASECURITYMARKETDATAQUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include <Beam/Queues/QueueWriter.hpp>
#include <Beam/Routines/Routine.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/MarketQuote.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus {
  typedef Beam::Queries::SequencedValue<BboQuote> SequencedBboQuote;
  typedef Beam::Queries::SequencedValue<BookQuote> SequencedBookQuote;
  typedef Beam::Queries::SequencedValue<MarketQuote> SequencedMarketQuote;
  typedef Beam::Queries::SequencedValue<TimeAndSale> SequencedTimeAndSale;
  typedef Beam::Queries::IndexedValue<BboQuote, Security> SecurityBboQuote;
  typedef Beam::Queries::IndexedValue<BookQuote, Security> SecurityBookQuote;
  typedef Beam::Queries::IndexedValue<MarketQuote, Security>
    SecurityMarketQuote;
  typedef Beam::Queries::IndexedValue<TimeAndSale, Security>
    SecurityTimeAndSale;
  typedef Beam::Queries::SequencedValue<SecurityBboQuote>
    SequencedSecurityBboQuote;
  typedef Beam::Queries::SequencedValue<SecurityBookQuote>
    SequencedSecurityBookQuote;
  typedef Beam::Queries::SequencedValue<SecurityMarketQuote>
    SequencedSecurityMarketQuote;
  typedef Beam::Queries::SequencedValue<SecurityTimeAndSale>
    SequencedSecurityTimeAndSale;

namespace MarketDataService {

  //! Defines the type of query used to receive a Security's market data.
  using SecurityMarketDataQuery = Beam::Queries::BasicQuery<Security>;

  //! Builds a SecurityMarketDataQuery for real time data with a snapshot
  //! containing the most recent value.
  /*!
    \param security The Security to query.
  */
  inline SecurityMarketDataQuery QueryRealTimeWithSnapshot(
      Security security) {
    SecurityMarketDataQuery query;
    query.SetIndex(std::move(security));
    query.SetRange(Beam::Queries::Range::Total());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::TAIL, 1);
    query.SetInterruptionPolicy(
      Beam::Queries::InterruptionPolicy::IGNORE_CONTINUE);
    return query;
  }

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
          snapshot = snapshotQueue->Top();
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
