#ifndef NEXUS_REPORT_SUBMISSION_HPP
#define NEXUS_REPORT_SUBMISSION_HPP
#include <stdexcept>
#include "WebPortal/ReportJob.hpp"
#include "WebPortal/ReportSchedule.hpp"

namespace Nexus {

  /** The inputs used to generate a report. */
  struct ReportSubmission {

    /** The report definition identifier. */
    std::string m_report_type;

    /** The supplied parameter values. */
    Beam::JsonObject m_parameters;

    /** The accounts/groups receiving access to the result. */
    std::vector<Beam::DirectoryEntry> m_recipients;
  };

  /** The editable settings of a scheduled report. */
  struct ReportScheduleSubmission {

    /** The report type, parameters, and recipients. */
    ReportSubmission m_report;

    /** The configured first run's calendar date and time. */
    boost::posix_time::ptime m_start_time;

    /** The repeat interval, absent for a one-time schedule. */
    std::optional<ReportSchedule::Interval> m_repeat_interval;

    /** The submitting browser's IANA timezone. */
    std::string m_time_zone;
  };

  /** Indicates that a report definition or job is unavailable to an account. */
  class ReportNotFoundException : public std::runtime_error {
    public:
      ReportNotFoundException();
  };

  /** Parses an account/group entry without performing a directory lookup. */
  Beam::DirectoryEntry parse_report_entry(const Beam::JsonValue& value);

  /** Parses a finite calendar date and time from a report input. */
  boost::posix_time::ptime parse_report_datetime(const Beam::JsonValue& value);

  /** Parses a positive calendar interval from a report input. */
  ReportSchedule::Interval parse_report_interval(const Beam::JsonValue& value);

  /** Validates edited settings and computes the schedule's next run. */
  ReportSchedule prepare_report_schedule(
    const ReportSchedule& schedule, const ReportScheduleSubmission& submission,
    const std::vector<ReportDefinition>& definitions,
    Beam::ServiceLocatorClient& client, boost::posix_time::ptime now);

  /** Resolves a submission into an authorized, prepared report job. */
  ReportJob prepare_report_job(const std::vector<ReportDefinition>& definitions,
    const Beam::DirectoryEntry& account, const ReportSubmission& submission,
    Beam::ServiceLocatorClient& client);

  /** Resolves distinct, readable accounts/groups for report sharing. */
  std::vector<Beam::DirectoryEntry> prepare_report_recipients(
    const std::vector<Beam::DirectoryEntry>& recipients,
    const Beam::DirectoryEntry& account, Beam::ServiceLocatorClient& client);

  /**
   * Checks current access to saved jobs and their referenced accounts/groups.
   * @param jobs The failed jobs to validate.
   * @param definitions The currently accessible report definitions.
   * @param client The client for directory and permission lookups.
   */
  void validate_report_retries(const std::vector<ReportJob>& jobs,
    const std::vector<ReportDefinition>& definitions,
    Beam::ServiceLocatorClient& client);

  /** Applies defaults and validates report parameters for an account. */
  Beam::JsonObject prepare_report_parameters(const ReportDefinition& definition,
    const Beam::JsonObject& parameters, const Beam::DirectoryEntry& account,
    Beam::ServiceLocatorClient& client);

  /** Resolves a readable account/group entry, optionally permitting global. */
  Beam::DirectoryEntry resolve_report_entry(const Beam::JsonValue& value,
    const Beam::DirectoryEntry& account, Beam::ServiceLocatorClient& client,
    bool allow_global);

  /** Expands argument templates into individual command-line arguments. */
  std::vector<std::string> make_report_arguments(
    const ReportDefinition& definition, const Beam::JsonObject& parameters);
}

#endif
