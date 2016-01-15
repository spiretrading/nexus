#ifndef NEXUS_CHARTINGSERVICES_HPP
#define NEXUS_CHARTINGSERVICES_HPP
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVariant.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/ChartingService/ChartingService.hpp"
#include "Nexus/ChartingService/ChartingQueryResults.hpp"
#include "Nexus/ChartingService/SecurityChartingQuery.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus {
namespace ChartingService {
  BEAM_DEFINE_SERVICES(ChartingServices,

    /*! \interface Nexus::ChartingServices::QuerySecurityService
        \brief Queries a Security over a time range.
        \param query <code>Nexus::ChartingService::SecurityChartingQuery</code>
               The query to submit.
        \param query_id A unique id to identify the query by.
        \return <code>Nexus::ChartingService::SecurityChartingQueryResult</code>
                A snapshot of the query and its unique id.
    */
    //! \cond
    (QuerySecurityService, "Nexus.ChartingServices.QuerySecurityService",
      SecurityChartingQueryResult, SecurityChartingQuery, query, int, query_id),
    //! \endcond

    /*! \interface Nexus::ChartingServices::LoadSecurityTimePriceSeriesService
        \brief Loads a time/price series for a Security.
        \param security <code>Security</code> The Security to load the series
               for.
        \param start_time <code>boost::posix_time::ptime</code> The series start
               time (inclusive).
        \param end_time <code>boost::posix_time::ptime</code> The series end
               time (inclusive).
        \param interval <code>boost::posix_time::time_duration</code> The time
               interval per Candlestick.
        \return <code>TimePriceSeries</code> The Security's time/price series
                with the specified parameters.
    */
    //! \cond
    (LoadSecurityTimePriceSeriesService,
      "Nexus.ChartingServices.LoadSecurityTimePriceSeriesService",
      TimePriceQueryResult, Security, security,
      boost::posix_time::ptime, start_time, boost::posix_time::ptime, end_time,
      boost::posix_time::time_duration, interval));
    //! \endcond

  BEAM_DEFINE_MESSAGES(ChartingMessages,

    /*! \interface Nexus::ChartingService::SecurityQueryMessage
        \brief Sends an update to a Security query.
        \param query_id The id of the query being updated.
        \param timestamp The value's Timestamp.
        \param value The updated QueryValue.
    */
    //! \cond
    (SecurityQueryMessage, "Nexus.ChartingService.SecurityQueryMessage", int,
      query_id, Queries::SequencedQueryVariant, value),
    //! \endcond

    /*! \interface Nexus::ChartingService::EndSecurityQueryMessage
        \brief Terminates a previous Security query.
        \param query_id <code>int</code> The id of query to end.
    */
    //! \cond
    (EndSecurityQueryMessage, "Nexus.ChartingService.EndSecurityQueryMessage",
      int, query_id));
    //! \endcond
}
}

#endif
