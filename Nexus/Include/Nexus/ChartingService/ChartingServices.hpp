#ifndef NEXUS_CHARTING_SERVICES_HPP
#define NEXUS_CHARTING_SERVICES_HPP
#include <Beam/Queries/QueryResult.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/ChartingService/TickerChartingQuery.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus {
  using TickerChartingQueryResult = Beam::QueryResult<SequencedQueryVariant>;
  BEAM_DEFINE_RECORD(PriceQueryResult, (Beam::Sequence, start),
    (Beam::Sequence, end), (PriceSeries, series));

  /** Standard name for the charting service. */
  inline const auto CHARTING_SERVICE_NAME = std::string("charting_service");

  BEAM_DEFINE_SERVICES(charting_services,

    /**
     * Queries a Ticker over a time range.
     * @param query The query to submit.
     * @param query_id A unique id to identify the query by.
     * @return A snapshot of the query and its unique id.
     */
    (QueryTickerService, "Nexus.ChartingServices.QueryTickerService",
      TickerChartingQueryResult, (TickerChartingQuery, query), (int, query_id)),

    /**
     * Loads a time/price series for a Ticker.
     * @param ticker The Ticker to load the series for.
     * @param start_time The series start time (inclusive).
     * @param end_time The series end time (inclusive).
     * @param interval The time interval per Candlestick.
     * @return The Ticker's time/price series with the specified parameters.
     */
    (LoadTickerPriceSeriesService,
      "Nexus.ChartingServices.LoadTickerPriceSeriesService",
      PriceQueryResult, (Ticker, ticker),
      (boost::posix_time::ptime, start_time),
      (boost::posix_time::ptime, end_time),
      (boost::posix_time::time_duration, interval)));

  BEAM_DEFINE_MESSAGES(charting_messages,

    /**
     * Sends an update to a Ticker query.
     * @param query_id The id of the query being updated.
     * @param timestamp The value's Timestamp.
     * @param value The updated QueryValue.
     */
    (TickerQueryMessage, "Nexus.ChartingService.TickerQueryMessage",
      (int, query_id), (SequencedQueryVariant, value)),

    /**
     * Terminates a previous Ticker query.
     * @param query_id The id of query to end.
     */
    (EndTickerQueryMessage, "Nexus.ChartingService.EndTickerQueryMessage",
      (int, query_id)));
}

#endif
