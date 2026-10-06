#ifndef NEXUS_REPORT_DETAIL_HPP
#define NEXUS_REPORT_DETAIL_HPP
#include <Beam/WebServices/Uri.hpp>
#include "WebPortal/ReportJob.hpp"

namespace Nexus {

  /** The public details of a completed report. */
  struct ReportDetail {

    /** A labeled parameter value. */
    struct Parameter {

      /** The parameter's display label at submission. */
      std::string m_label;

      /** The formatted value used to generate the report. */
      std::string m_value;
    };

    /** The report identifier. */
    std::string m_id;

    /** The report type's display name at submission. */
    std::string m_title;

    /** The configured parameter values in definition order. */
    std::vector<Parameter> m_parameters;

    /** The authenticated URL from which to download the generated file. */
    Beam::Uri m_file_path;
  };

  /** Formats configured parameter labels and values in definition order. */
  std::vector<ReportDetail::Parameter> format_report_parameters(
    const ReportDefinition& definition, const Beam::JsonObject& parameters);

  /** Makes public report details from a saved job. */
  ReportDetail make_report_detail(const ReportJob& job);
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ReportDetail::Parameter> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::ReportDetail::Parameter& value,
        unsigned int version) const {
      shuttle.shuttle("label", value.m_label);
      shuttle.shuttle("value", value.m_value);
    }
  };

  template<>
  struct Shuttle<Nexus::ReportDetail> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::ReportDetail& value, unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("title", value.m_title);
      shuttle.shuttle("parameters", value.m_parameters);
      shuttle.shuttle("file_path", value.m_file_path);
    }
  };
}

#endif
