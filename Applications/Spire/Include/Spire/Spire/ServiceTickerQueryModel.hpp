#ifndef SPIRE_SERVICE_TICKER_QUERY_MODEL_HPP
#define SPIRE_SERVICE_TICKER_QUERY_MODEL_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Spire/Spire/QueryModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Implements a QueryModel by submitting queries for tickers to a market data
   * client.
   */
  class ServiceTickerQueryModel : public QueryModel<Nexus::TickerInfo> {
    public:

      /**
       * Constructs a ServiceTickerQueryModel.
       * @param market_data_client The MarketDataClient to submit queries to.
       */
      explicit ServiceTickerQueryModel(
        Nexus::MarketDataClient market_data_client);

      boost::optional<Nexus::TickerInfo> parse(const QString& query) override;
      QtPromise<std::vector<Nexus::TickerInfo>>
        submit(const QString& query) override;

    private:
      Nexus::MarketDataClient m_market_data_client;
      Beam::SynchronizedUnorderedMap<Nexus::Ticker, Nexus::TickerInfo> m_info;
  };
}

#endif
