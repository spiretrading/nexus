#ifndef NEXUS_CHARTING_SERVICES_HPP
#define NEXUS_CHARTING_SERVICES_HPP
#include <Beam/Queries/QueryResult.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus::ChartingService {
  using SecurityChartingQueryResult =
    Beam::Queries::QueryResult<Queries::SequencedQueryVariant>;
  BEAM_DEFINE_RECORD(TimePriceQueryResult, Beam::Queries::Sequence, start,
    Beam::Queries::Sequence, end, TechnicalAnalysis::TimePriceSeries, series);

  /** Standard name for the charting service. */
  inline const auto SERVICE_NAME = std::string("charting_service");

  BEAM_DEFINE_SERVICES(ChartingServices,

    /**
     * Queries a Security over a time range.
     * @param query The query to submit.
     * @param query_id A unique id to identify the query by.
     * @return A snapshot of the query and its unique id.
     */
    (QuerySecurityService, "Nexus.ChartingServices.QuerySecurityService",
      SecurityChartingQueryResult, SecurityChartingQuery, query, int, query_id),

    /**
     * Loads a time/price series for a Security.
     * @param security The Security to load the series for.
     * @param start_time The series start time (inclusive).
     * @param end_time The series end time (inclusive).
     * @param interval The time interval per Candlestick.
     * @return The Security's time/price series with the specified parameters.
     */
    (LoadSecurityTimePriceSeriesService,
      "Nexus.ChartingServices.LoadSecurityTimePriceSeriesService",
      TimePriceQueryResult, Security, security,
      boost::posix_time::ptime, start_time, boost::posix_time::ptime, end_time,
      boost::posix_time::time_duration, interval));

  BEAM_DEFINE_MESSAGES(ChartingMessages,

    /**
     * Sends an update to a Security query.
     * @param query_id The id of the query being updated.
     * @param timestamp The value's Timestamp.
     * @param value The updated QueryValue.
     */
    (SecurityQueryMessage, "Nexus.ChartingService.SecurityQueryMessage", int,
      query_id, Queries::SequencedQueryVariant, value),

    /**
     * Terminates a previous Security query.
     * @param query_id The id of query to end.
     */
    (EndSecurityQueryMessage, "Nexus.ChartingService.EndSecurityQueryMessage",
      int, query_id));
}

#endif
