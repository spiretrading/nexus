#ifndef NEXUS_MARKET_DATA_CLIENT_UTILITIES_HPP
#define NEXUS_MARKET_DATA_CLIENT_UTILITIES_HPP
#include <memory>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/QueryTypes.hpp"

namespace Nexus::MarketDataService {
namespace Details {
  template<typename MarketDataType, typename MarketDataClient>
  struct QueryMarketDataClientOperator {};

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedOrderImbalance,
      MarketDataClient> {
    void operator ()(MarketDataClient& client, const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) const {
      client.QueryOrderImbalances(query, std::move(queue));
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedTimeAndSale, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) const {
      client.QueryTimeAndSales(query, std::move(queue));
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedBookQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue) const {
      client.QueryBookQuotes(query, std::move(queue));
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedMarketQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedMarketQuote> queue) const {
      client.QueryMarketQuotes(query, std::move(queue));
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedBboQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue) const {
      client.QueryBboQuotes(query, std::move(queue));
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<OrderImbalance, MarketDataClient> {
    void operator ()(MarketDataClient& client, const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue) const {
      client.QueryOrderImbalances(query, std::move(queue));
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<TimeAndSale, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue) const {
      client.QueryTimeAndSales(query, std::move(queue));
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<BookQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue) const {
      client.QueryBookQuotes(query, std::move(queue));
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<MarketQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<MarketQuote> queue) const {
      client.QueryMarketQuotes(query, std::move(queue));
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<BboQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue) const {
      client.QueryBboQuotes(query, std::move(queue));
    }
  };
}

  /**
   * Submits a query for market data.
   * @param marketDataClient The MarketDataClient used to submit the query.
   * @param query The query to submit.
   * @param queue The queue that will store the result of the query.
   */
  template<typename MarketDataClient, typename Query, typename Q,
    typename U>
  void QueryMarketDataClient(MarketDataClient& client,
      const Query& query, Beam::ScopedQueueWriter<Q, U> queue) {
    return Details::QueryMarketDataClientOperator<
      typename Beam::ScopedQueueWriter<Q, U>::Target, MarketDataClient>()(
      client, query, std::move(queue));
  }
}

#endif
