#ifndef SPIRE_TICKER_INFO_TO_TICKER_QUERY_MODEL_HPP
#define SPIRE_TICKER_INFO_TO_TICKER_QUERY_MODEL_HPP
#include "Nexus/Definitions/TickerInfo.hpp"
#include "Spire/Spire/QueryModel.hpp"

namespace Spire {

  /** Represents a QueryModel for the TickerInfo. */
  using TickerInfoQueryModel = QueryModel<Nexus::TickerInfo>;

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
        std::shared_ptr<TickerInfoQueryModel> source);

      /** Returns the source model. */
      const std::shared_ptr<TickerInfoQueryModel>& get_source() const;

      boost::optional<Nexus::Ticker> parse(const QString& query) override;

      QtPromise<std::vector<Nexus::Ticker>>
        submit(const QString& query) override;

    private:
      std::shared_ptr<TickerInfoQueryModel> m_source;
  };
}

#endif
