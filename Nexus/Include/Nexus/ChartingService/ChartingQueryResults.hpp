#ifndef NEXUS_CHARTINGQUERYRESULTS_HPP
#define NEXUS_CHARTINGQUERYRESULTS_HPP
#include <Beam/Queries/QueryResult.hpp>
#include <Beam/Queries/Sequence.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVariant.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/ChartingService/ChartingService.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus {
namespace ChartingService {
  using SecurityChartingQueryResult = Beam::Queries::QueryResult<
    Queries::SequencedQueryVariant>;
  BEAM_DEFINE_RECORD(TimePriceQueryResult, Beam::Queries::Sequence, start,
    Beam::Queries::Sequence, end, TechnicalAnalysis::TimePriceSeries, series);
}
}

#endif
