#ifndef NEXUS_GENERATED_REPORT_HPP
#define NEXUS_GENERATED_REPORT_HPP
#include <cstdint>
#include <Beam/WebServices/Uri.hpp>
#include "WebPortal/ReportJob.hpp"

namespace Nexus {

  /** A completed report available for viewing. */
  struct GeneratedReport {

    /** The report identifier. */
    std::string m_id;

    /** The report type's display name at submission. */
    std::string m_type;

    /** The configured parameter values in definition order. */
    std::vector<std::string> m_parameters;

    /** The link to the report detail page. */
    Beam::Uri m_url;

    /** The UTC date when the report finished generating. */
    boost::gregorian::date m_date_created;
  };

  /** Selects one page of completed reports. */
  struct GeneratedReportQuery {

    /** The maximum number of reports in a page. */
    static constexpr auto PAGE_SIZE = std::size_t(50);

    /** The column used to order reports. */
    enum class Column {

      /** The report type's display name. */
      TYPE,

      /** The displayed parameter values. */
      PARAMETERS,

      /** The UTC date when the report finished generating. */
      DATE_CREATED
    };

    /** The direction in which reports are ordered. */
    enum class Order {

      /** Most recently generated first. */
      NONE,

      /** Increasing column values. */
      ASCENDING,

      /** Decreasing column values. */
      DESCENDING
    };

    /** Text to match within the type, parameter values, or displayed date. */
    std::string m_query;

    /** The inclusive first generation date, or no lower bound. */
    std::optional<boost::gregorian::date> m_start_date;

    /** The inclusive last generation date, or no upper bound. */
    std::optional<boost::gregorian::date> m_end_date;

    /** The column to sort. */
    Column m_column = Column::DATE_CREATED;

    /** The requested ordering. */
    Order m_order = Order::NONE;

    /** The zero-based page index. */
    std::uint32_t m_page_index = 0;
  };

  /** One page of completed reports and its filtered count. */
  struct GeneratedReports {

    /** Whether the account has no accessible reports before filtering. */
    bool m_is_empty = true;

    /** The number of matching reports across all pages. */
    std::size_t m_filtered_count = 0;

    /** The requested page of reports. */
    std::vector<GeneratedReport> m_reports;
  };

  /** Selects completed reports owned by or shared with an account. */
  GeneratedReports query_generated_reports(const std::vector<ReportJob>& jobs,
    const Beam::DirectoryEntry& account, const GeneratedReportQuery& query,
    Beam::ServiceLocatorClient& client);
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::GeneratedReport> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::GeneratedReport& value, unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("parameters", value.m_parameters);
      shuttle.shuttle("url", value.m_url);
      shuttle.shuttle("date_created", value.m_date_created);
    }
  };
}

#endif
