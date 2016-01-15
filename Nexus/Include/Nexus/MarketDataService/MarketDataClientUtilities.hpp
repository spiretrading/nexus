#ifndef NEXUS_MARKETDATACLIENTUTILITIES_HPP
#define NEXUS_MARKETDATACLIENTUTILITIES_HPP
#include <memory>
#include <Beam/Queues/QueueWriter.hpp>
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/QueryTypes.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Details {
  template<typename MarketDataType, typename MarketDataClient>
  struct QueryMarketDataClientOperator {};

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedOrderImbalance,
      MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<
        SequencedOrderImbalance>>& queue) const {
      client.QueryOrderImbalances(query, queue);
    };
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedTimeAndSale, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<
        SequencedTimeAndSale>>& queue) const {
      client.QueryTimeAndSales(query, queue);
    };
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedBookQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<
        SequencedBookQuote>>& queue) const {
      client.QueryBookQuotes(query, queue);
    };
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedMarketQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<
        SequencedMarketQuote>>& queue) const {
      client.QueryMarketQuotes(query, queue);
    };
  };

  template<typename MarketDataClient>
  struct QueryMarketDataClientOperator<SequencedBboQuote, MarketDataClient> {
    void operator ()(MarketDataClient& client,
        const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<
        SequencedBboQuote>>& queue) const {
      client.QueryBboQuotes(query, queue);
    };
  };
}

  //! Submits a query for market data.
  /*!
    \param marketDataClient The MarketDataClient used to submit the query.
    \param query The query to submit.
    \param queue The queue that will store the result of the query.
  */
  template<typename MarketDataType, typename MarketDataClient>
  void QueryMarketDataClient(MarketDataClient& client,
      const typename MarketDataQueryType<MarketDataType>::type& query,
      const std::shared_ptr<Beam::QueueWriter<MarketDataType>>& queue) {
    return Details::QueryMarketDataClientOperator<
      MarketDataType, MarketDataClient>()(client, query, queue);
  }
}
}

#endif
