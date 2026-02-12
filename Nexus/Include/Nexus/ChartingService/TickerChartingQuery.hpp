#ifndef NEXUS_TICKER_CHARTING_QUERY_HPP
#define NEXUS_TICKER_CHARTING_QUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/ExpressionQuery.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/MarketDataService/MarketDataType.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {

  /** Queries for charting data over a particular Ticker. */
  class TickerChartingQuery :
      public Beam::BasicQuery<Ticker>, public Beam::ExpressionQuery {
    public:

      /** Returns the type of data to query. */
      MarketDataType get_market_data_type() const;

      /** Sets the type of data to query. */
      void set_market_data_type(MarketDataType type);

    private:
      friend struct Beam::DataShuttle;
      MarketDataType m_type;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  inline MarketDataType TickerChartingQuery::get_market_data_type() const {
    return m_type;
  }

  inline void TickerChartingQuery::set_market_data_type(MarketDataType type) {
    m_type = type;
  }

  template<Beam::IsShuttle S>
  void TickerChartingQuery::shuttle(S& shuttle, unsigned int version) {
    Beam::BasicQuery<Ticker>::shuttle(shuttle, version);
    Beam::Shuttle<Beam::ExpressionQuery>()(shuttle, *this, version);
    shuttle.shuttle("type", m_type);
  }
}

#endif
