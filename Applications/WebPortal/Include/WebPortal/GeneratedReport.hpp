#ifndef NEXUS_GENERATED_REPORT_HPP
#define NEXUS_GENERATED_REPORT_HPP
#include <cstdint>
#include <Beam/WebServices/Uri.hpp>
#include "WebPortal/ReportJob.hpp"

namespace Nexus {

  /** A visible report job and its current execution state. */
  struct GeneratedReport {

    /** The displayed execution state. */
    enum class Status {

      /** Queued or currently running. */
      GENERATING,

      /** Execution failed. */
      FAILED,

      /** The generated report is available. */
      READY
    };

    /** The report identifier. */
    std::string m_id;

    /** The report type's display name at submission. */
    std::string m_type;

    /** The configured parameter values in definition order. */
    std::vector<std::string> m_parameters;

    /** The link to the report detail page. */
    Beam::Uri m_url;

    /** The UTC timestamp of the latest job update. */
    boost::posix_time::ptime m_date_modified;

    /** The displayed execution state. */
    Status m_status = Status::READY;
  };

  /** Selects one page of visible report jobs. */
  struct GeneratedReportQuery {

    /** The maximum number of reports in a page. */
    static constexpr auto PAGE_SIZE = std::size_t(50);

    /** The column used to order reports. */
    enum class Column {

      /** The report type's display name. */
      TYPE,

      /** The displayed parameter values. */
      PARAMETERS,

      /** The displayed execution state. */
      STATUS,

      /** The timestamp of the latest job update. */
      DATE_MODIFIED
    };

    /** The direction in which reports are ordered. */
    enum class Order {

      /** Most recently modified first. */
      NONE,

      /** Increasing column values. */
      ASCENDING,

      /** Decreasing column values. */
      DESCENDING
    };

    /** Text to match within the type, parameter values, or displayed date. */
    std::string m_query;

    /** The inclusive lower bound on the last-modified date, if any. */
    std::optional<boost::gregorian::date> m_start_date;

    /** The inclusive upper bound on the last-modified date, if any. */
    std::optional<boost::gregorian::date> m_end_date;

    /** The column to sort. */
    Column m_column = Column::DATE_MODIFIED;

    /** The requested ordering. */
    Order m_order = Order::NONE;

    /** The zero-based page index. */
    std::uint32_t m_page_index = 0;
  };

  /** One page of visible report jobs and its filtered count. */
  struct GeneratedReports {

    /** Whether the account has no accessible reports before filtering. */
    bool m_is_empty = true;

    /** The number of matching reports across all pages. */
    std::size_t m_filtered_count = 0;

    /** The requested page of reports. */
    std::vector<GeneratedReport> m_reports;
  };

  /** Formats a report date for text searches. */
  std::string format_report_date(boost::gregorian::date value);

  /**
   * Selects owned report jobs and completed reports shared with an account.
   * Cancelled jobs are excluded.
   */
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
      shuttle.shuttle("date_modified", value.m_date_modified);
      shuttle.shuttle("status", value.m_status);
    }
  };
}

#endif
