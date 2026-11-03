#ifndef NEXUS_SCHEDULED_REPORT_HPP
#define NEXUS_SCHEDULED_REPORT_HPP
#include "WebPortal/ReportDetail.hpp"
#include "WebPortal/ReportSchedule.hpp"

namespace Nexus {

  /** The public display values of a scheduled report. */
  struct ScheduledReport {

    /** The schedule identifier. */
    std::string m_id;

    /** The report type's saved display name. */
    std::string m_type;

    /** The configured parameter labels and values in definition order. */
    std::vector<ReportDetail::Parameter> m_parameters;

    /** Whether the schedule repeats. */
    bool m_is_repeating = false;

    /** The upcoming run date, or original date for a one-time schedule. */
    boost::gregorian::date m_run_date;
  };

  /** Selects one page of an account's scheduled reports. */
  struct ScheduledReportQuery {

    /** The maximum number of schedules in a page. */
    static constexpr auto PAGE_SIZE = std::size_t(50);

    /** Text to match within the displayed title, parameters, or run date. */
    std::string m_query;

    /** The zero-based page index. */
    std::uint32_t m_page_index = 0;
  };

  /** One page of scheduled reports and its filtered count. */
  struct ScheduledReports {

    /** Whether the account owns no schedules before filtering. */
    bool m_is_empty = true;

    /** The number of matching schedules across all pages. */
    std::size_t m_filtered_count = 0;

    /** The requested page of schedules. */
    std::vector<ScheduledReport> m_schedules;
  };

  /** Builds the public display values of a saved schedule. */
  ScheduledReport make_scheduled_report(const ReportSchedule& schedule);

  /** Queries schedules owned by an account, newest first. */
  ScheduledReports query_scheduled_reports(
    const std::vector<ReportSchedule>& schedules,
    const Beam::DirectoryEntry& account, const ScheduledReportQuery& query);
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ScheduledReport> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::ScheduledReport& value, unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("parameters", value.m_parameters);
      shuttle.shuttle("repeats", value.m_is_repeating);
      shuttle.shuttle("run_date", value.m_run_date);
    }
  };
}

#endif
