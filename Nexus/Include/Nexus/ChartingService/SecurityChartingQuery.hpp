#ifndef NEXUS_SECURITY_CHARTING_QUERY_HPP
#define NEXUS_SECURITY_CHARTING_QUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/ExpressionQuery.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/MarketDataService/MarketDataType.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {

  /** Queries for charting data over a particular Security. */
  class SecurityChartingQuery :
      public Beam::BasicQuery<Security>, public Beam::ExpressionQuery {
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

  inline MarketDataType SecurityChartingQuery::get_market_data_type() const {
    return m_type;
  }

  inline void SecurityChartingQuery::set_market_data_type(MarketDataType type) {
    m_type = type;
  }

  template<Beam::IsShuttle S>
  void SecurityChartingQuery::shuttle(S& shuttle, unsigned int version) {
    Beam::BasicQuery<Security>::shuttle(shuttle, version);
    Beam::Shuttle<Beam::ExpressionQuery>()(shuttle, *this, version);
    shuttle.shuttle("type", m_type);
  }
}

#endif
