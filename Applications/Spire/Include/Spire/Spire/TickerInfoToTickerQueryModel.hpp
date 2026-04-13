#ifndef SPIRE_TICKER_INFO_TO_TICKER_QUERY_MODEL_HPP
#define SPIRE_TICKER_INFO_TO_TICKER_QUERY_MODEL_HPP
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/Definitions/TickerInfo.hpp"
#include "Spire/Spire/QueryModel.hpp"

namespace Spire {

  /**
   * Implements a QueryModel over Tickers by adapting a QueryModel<TickerInfo>.
   */
  class TickerInfoToTickerQueryModel : public QueryModel<Nexus::Ticker> {
    public:

      /**
       * Constructs a TickerInfoToTickerQueryModel.
       * @param source The source QueryModel providing TickerInfo results.
       */
      explicit TickerInfoToTickerQueryModel(
        std::shared_ptr<QueryModel<Nexus::TickerInfo>> source);

      /** Returns the source model. */
      const std::shared_ptr<QueryModel<Nexus::TickerInfo>>& get_source() const;

      boost::optional<Nexus::Ticker> parse(const QString& query) override;

      QtPromise<std::vector<Nexus::Ticker>>
        submit(const QString& query) override;

    private:
      std::shared_ptr<QueryModel<Nexus::TickerInfo>> m_source;
  };
}

#endif
