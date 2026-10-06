#include "WebPortal/ReportDetail.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

ReportDetail Nexus::make_report_detail(const ReportJob& job) {
  auto parameters = std::vector<ReportDetail::Parameter>();
  for(auto& parameter : job.m_definition.m_parameters) {
    auto value = job.m_parameters.get(parameter.m_name);
    if(value && !get<JsonNull>(&*value)) {
      parameters.emplace_back(
        parameter.m_label, format_report_parameter(parameter, *value));
    }
  }
  return ReportDetail(job.m_id, job.m_definition.m_name, std::move(parameters),
    Uri("/api/reporting_service/download_report?id=" + uri_encode(job.m_id)));
}
