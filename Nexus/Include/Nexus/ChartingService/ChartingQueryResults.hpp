#ifndef NEXUS_CHARTING_QUERY_RESULTS_HPP
#define NEXUS_CHARTING_QUERY_RESULTS_HPP
#include <Beam/Queries/QueryResult.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include "Nexus/ChartingService/ChartingService.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus::ChartingService {
  using SecurityChartingQueryResult = Beam::Queries::QueryResult<
    Queries::SequencedQueryVariant>;
  BEAM_DEFINE_RECORD(TimePriceQueryResult, Beam::Queries::Sequence, start,
    Beam::Queries::Sequence, end, TechnicalAnalysis::TimePriceSeries, series);
}

#endif
