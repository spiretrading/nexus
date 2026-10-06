#ifndef NEXUS_REPORT_JOB_HPP
#define NEXUS_REPORT_JOB_HPP
#include <ostream>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include "WebPortal/ReportDefinition.hpp"

namespace Nexus {

  /** A submitted report and the result of executing it. */
  struct ReportJob {

    /** The execution state of a report job. */
    enum class Status {

      /** Waiting to execute. */
      QUEUED,

      /** Currently executing. */
      RUNNING,

      /** Finished successfully. */
      COMPLETED,

      /** Could not complete successfully. */
      FAILED,

      /** Cancelled by the submitting account. */
      CANCELLED
    };

    /** The unique job identifier. */
    std::string m_id;

    /** The account that submitted the report. */
    Beam::DirectoryEntry m_account;

    /** The accounts/groups with which the result is shared. */
    std::vector<Beam::DirectoryEntry> m_recipients;

    /** The definition at submission time. */
    ReportDefinition m_definition;

    /** The normalized parameter values. */
    Beam::JsonObject m_parameters;

    /** The expanded command-line arguments. */
    std::vector<std::string> m_arguments;

    /** The UTC submission time. */
    boost::posix_time::ptime m_created;

    /** The UTC completion time, or not-a-date-time while pending. */
    boost::posix_time::ptime m_completed;

    /** The latest state change time, absent in older stored jobs. */
    std::optional<boost::posix_time::ptime> m_modified;

    /** The execution state. */
    Status m_status = Status::QUEUED;

    /** The process exit code, when available. */
    std::optional<int> m_exit_code;

    /** The failure diagnostic, when available. */
    std::string m_error;
  };

  /** Formats a normalized parameter value for display. */
  std::string format_report_parameter(
    const ReportParameterDefinition& parameter, const Beam::JsonValue& value);

  /** Formats configured parameter values in definition order for display. */
  std::vector<std::string> format_report_parameters(const ReportJob& job);

  inline std::ostream& operator <<(
      std::ostream& out, ReportJob::Status status) {
    if(status == ReportJob::Status::QUEUED) {
      return out << "QUEUED";
    } else if(status == ReportJob::Status::RUNNING) {
      return out << "RUNNING";
    } else if(status == ReportJob::Status::COMPLETED) {
      return out << "COMPLETED";
    } else if(status == ReportJob::Status::FAILED) {
      return out << "FAILED";
    } else if(status == ReportJob::Status::CANCELLED) {
      return out << "CANCELLED";
    } else {
      return out << "UNKNOWN(" << static_cast<int>(status) << ')';
    }
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ReportJob> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::ReportJob& value, unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("account", value.m_account);
      shuttle.shuttle("recipients", value.m_recipients);
      shuttle.shuttle("definition", value.m_definition);
      auto parameters = JsonValue(value.m_parameters);
      shuttle.shuttle("parameters", parameters);
      if constexpr(IsReceiver<S>) {
        value.m_parameters = get<JsonObject>(parameters);
      }
      shuttle.shuttle("arguments", value.m_arguments);
      shuttle.shuttle("created", value.m_created);
      shuttle.shuttle("completed", value.m_completed);
      shuttle.shuttle("modified", value.m_modified);
      shuttle.shuttle("status", value.m_status);
      shuttle.shuttle("exit_code", value.m_exit_code);
      shuttle.shuttle("error", value.m_error);
    }
  };
}

#endif
