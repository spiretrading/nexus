#ifndef NEXUS_REPORT_SCHEDULE_HPP
#define NEXUS_REPORT_SCHEDULE_HPP
#include <cstdint>
#include "WebPortal/ReportJob.hpp"

namespace Nexus {

  /** A saved report configuration and its calendar schedule. */
  struct ReportSchedule {

    /** The calendar interval between report runs. */
    struct Interval {

      /** The supported calendar units. */
      enum class Unit {

        /** Calendar days. */
        DAY,

        /** Calendar weeks. */
        WEEK,

        /** Calendar months. */
        MONTH,

        /** Calendar years. */
        YEAR
      };

      /** The number of calendar units between runs. */
      std::uint32_t m_count = 1;

      /** The calendar unit. */
      Unit m_unit = Unit::DAY;
    };

    /** The schedule identifier. */
    std::string m_id;

    /** The account that owns the schedule. */
    Beam::DirectoryEntry m_account;

    /** The report definition saved with the schedule. */
    ReportDefinition m_definition;

    /** The normalized parameter values. */
    Beam::JsonObject m_parameters;

    /** The accounts/groups receiving the generated reports. */
    std::vector<Beam::DirectoryEntry> m_recipients;

    /** The UTC creation time. */
    boost::posix_time::ptime m_created;

    /** The configured first run's calendar date and time. */
    boost::posix_time::ptime m_start_time;

    /** The next recurring run, or the original run for a one-time schedule. */
    boost::posix_time::ptime m_run_time;

    /** The repeat interval, absent for a one-time schedule. */
    std::optional<Interval> m_repeat_interval;

    /** The IANA timezone for calendar times. */
    std::string m_time_zone = "UTC";

    /** The occurrence whose job and next run are being committed. */
    std::optional<std::string> m_pending_job_id;

    /** The final occurrence's job, while waiting for it to finish. */
    std::optional<std::string> m_job_id;
  };

  /** Converts a calendar date/time between IANA timezones. */
  boost::posix_time::ptime convert_report_time(boost::posix_time::ptime value,
    const std::string& source, const std::string& destination);

  /** Finds the next recurring run after now, or a one-time schedule's start. */
  boost::posix_time::ptime next_report_run(
    const ReportSchedule& schedule, boost::posix_time::ptime now);
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ReportSchedule::Interval> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::ReportSchedule::Interval& value,
        unsigned int version) const {
      shuttle.shuttle("count", value.m_count);
      shuttle.shuttle("unit", value.m_unit);
    }
  };

  template<>
  struct Shuttle<Nexus::ReportSchedule> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::ReportSchedule& value, unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("account", value.m_account);
      shuttle.shuttle("definition", value.m_definition);
      shuttle.shuttle("parameters", value.m_parameters);
      shuttle.shuttle("recipients", value.m_recipients);
      shuttle.shuttle("created", value.m_created);
      shuttle.shuttle("start_time", value.m_start_time);
      shuttle.shuttle("run_time", value.m_run_time);
      shuttle.shuttle("repeat_interval", value.m_repeat_interval);
      shuttle.shuttle("time_zone", value.m_time_zone);
      shuttle.shuttle("pending_job_id", value.m_pending_job_id);
      shuttle.shuttle("job_id", value.m_job_id);
    }
  };
}

#endif
