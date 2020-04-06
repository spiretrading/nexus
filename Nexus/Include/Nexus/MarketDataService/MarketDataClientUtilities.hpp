#ifndef NEXUS_MARKET_DATA_CLIENT_UTILITIES_HPP
#define NEXUS_MARKET_DATA_CLIENT_UTILITIES_HPP
#include <memory>
#include <Beam/Queues/QueueWriter.hpp>
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
        const std::shared_ptr<Beam::QueueWriter<
        SequencedOrderImbalance>>& queue) const {
      client.QueryOrderImbalances(query, queue);
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedTimeAndSale, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue)
        const {
      client.QueryTimeAndSales(query, queue);
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedBookQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue)
        const {
      client.QueryBookQuotes(query, queue);
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedMarketQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue)
        const {
      client.QueryMarketQuotes(query, queue);
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedBboQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue)
        const {
      client.QueryBboQuotes(query, queue);
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<OrderImbalance, MarketDataClient> {
    void operator ()(MarketDataClient& client, const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) const {
      client.QueryOrderImbalances(query, queue);
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<TimeAndSale, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) const {
      client.QueryTimeAndSales(query, queue);
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<BookQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) const {
      client.QueryBookQuotes(query, queue);
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<MarketQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) const {
      client.QueryMarketQuotes(query, queue);
    }
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<BboQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) const {
      client.QueryBboQuotes(query, queue);
    }
  };
}

  /**
   * Submits a query for market data.
   * @param marketDataClient The MarketDataClient used to submit the query.
   * @param query The query to submit.
   * @param queue The queue that will store the result of the query.
   */
  template<typename MarketDataClient, typename Query, typename Queue>
  void QueryMarketDataClient(MarketDataClient& client,
      const Query& query, const std::shared_ptr<Queue>& queue) {
    return Details::QueryMarketDataClientOperator<typename Queue::Source,
      MarketDataClient>()(client, query, queue);
  }
}

#endif
