#ifndef NEXUS_SECURITY_CHARTING_QUERY_HPP
#define NEXUS_SECURITY_CHARTING_QUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/ExpressionQuery.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/ChartingService/ChartingService.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus::ChartingService {

  /** Queries for charting data over a particular Security. */
  class SecurityChartingQuery : public Beam::Queries::BasicQuery<Security>,
      public Beam::Queries::ExpressionQuery {
    public:

      /** Returns the type of data to query. */
      MarketDataService::MarketDataType GetMarketDataType() const;

      /** Sets the type of data to query. */
      void SetMarketDataType(MarketDataService::MarketDataType type);

    private:
      friend struct Beam::Serialization::DataShuttle;
      MarketDataService::MarketDataType m_type;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline MarketDataService::MarketDataType SecurityChartingQuery::
      GetMarketDataType() const {
    return m_type;
  }

  inline void SecurityChartingQuery::SetMarketDataType(
      MarketDataService::MarketDataType type) {
    m_type = type;
  }

  template<typename Shuttler>
  void SecurityChartingQuery::Shuttle(Shuttler& shuttle, unsigned int version) {
    Beam::Queries::BasicQuery<Security>::Shuttle(shuttle, version);
    Beam::Serialization::Shuttle<Beam::Queries::ExpressionQuery>()(shuttle,
      *this, version);
    shuttle.Shuttle("type", m_type);
  }
}

#endif
