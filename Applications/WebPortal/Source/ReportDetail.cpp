#include "WebPortal/ReportDetail.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

std::vector<ReportDetail::Parameter> Nexus::format_report_parameters(
    const ReportDefinition& definition, const JsonObject& parameters) {
  auto result = std::vector<ReportDetail::Parameter>();
  for(auto& parameter : definition.m_parameters) {
    auto value = parameters.get(parameter.m_name);
    if(value && !std::get_if<JsonNull>(&*value)) {
      result.emplace_back(
        parameter.m_label, format_report_parameter(parameter, *value));
    }
  }
  return result;
}

ReportDetail Nexus::make_report_detail(const ReportJob& job) {
  return ReportDetail(job.m_id, job.m_definition.m_name,
    format_report_parameters(job.m_definition, job.m_parameters),
    Uri("/api/reporting_service/download_report?id=" + uri_encode(job.m_id)));
}
