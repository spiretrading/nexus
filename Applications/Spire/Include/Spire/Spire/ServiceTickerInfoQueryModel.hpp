#ifndef SPIRE_SERVICE_TICKER_INFO_QUERY_MODEL_HPP
#define SPIRE_SERVICE_TICKER_INFO_QUERY_MODEL_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Spire/Spire/QueryModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Implements a QueryModel by submitting queries for ticker info to a market
   * data client.
   */
  class ServiceTickerInfoQueryModel : public QueryModel<Nexus::TickerInfo> {
    public:

      /**
       * Constructs a ServiceTickerInfoQueryModel.
       * @param market_data_client The MarketDataClient to submit queries to.
       */
      explicit ServiceTickerInfoQueryModel(
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
