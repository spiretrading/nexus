#ifndef NEXUS_REPORT_ACTIVITY_HPP
#define NEXUS_REPORT_ACTIVITY_HPP
#include <cstdint>
#include "WebPortal/ReportJob.hpp"

namespace Nexus {

  /** A report job displayed on the activity page. */
  struct ReportActivity {

    /** The displayed execution state. */
    enum class Status {

      /** Queued or currently running. */
      GENERATING,

      /** Execution failed. */
      FAILED
    };

    /** The job identifier. */
    std::string m_id;

    /** The report type's display name at submission. */
    std::string m_type;

    /** The configured parameter values in definition order. */
    std::vector<std::string> m_parameters;

    /** The displayed execution state. */
    Status m_status = Status::GENERATING;

    /** The UTC date of the latest job update. */
    boost::gregorian::date m_date_modified;
  };

  /** Selects one page of report activity. */
  struct ReportActivityQuery {

    /** The maximum number of activities in a page. */
    static constexpr auto PAGE_SIZE = std::size_t(50);

    /** The column used to order activities. */
    enum class Column {

      /** The report type's display name. */
      TYPE,

      /** The displayed parameter values. */
      PARAMETERS,

      /** The displayed execution state. */
      STATUS,

      /** The UTC date of the latest job update. */
      DATE_MODIFIED
    };

    /** The direction in which activities are ordered. */
    enum class Order {

      /** Most recently submitted first. */
      NONE,

      /** Increasing column values. */
      ASCENDING,

      /** Decreasing column values. */
      DESCENDING
    };

    /** The column to sort. */
    Column m_column = Column::DATE_MODIFIED;

    /** The requested ordering. */
    Order m_order = Order::NONE;

    /** The zero-based page index. */
    std::uint32_t m_page_index = 0;
  };

  /** One page of activity and its total count. */
  struct ReportActivities {

    /** The total number of matching jobs across all pages. */
    std::size_t m_total_count = 0;

    /** The requested page of activities. */
    std::vector<ReportActivity> m_activities;
  };

  /** Selects activity belonging to an account from stored job snapshots. */
  ReportActivities query_report_activities(const std::vector<ReportJob>& jobs,
    const Beam::DirectoryEntry& account, const ReportActivityQuery& query);
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ReportActivity> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::ReportActivity& value, unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("parameters", value.m_parameters);
      shuttle.shuttle("status", value.m_status);
      shuttle.shuttle("date_modified", value.m_date_modified);
    }
  };
}

#endif
