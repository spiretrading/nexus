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
  auto status = [&] {
    if(job.m_status == ReportJob::Status::COMPLETED) {
      return ReportDetail::Status::READY;
    } else if(job.m_status == ReportJob::Status::STAGED ||
        job.m_status == ReportJob::Status::QUEUED ||
        job.m_status == ReportJob::Status::RUNNING) {
      return ReportDetail::Status::IN_PROGRESS;
    }
    return ReportDetail::Status::FAILED;
  }();
  auto file = [&] {
    if(status == ReportDetail::Status::READY) {
      return Uri("/api/reporting_service/download_report?id=" +
        uri_encode(job.m_id));
    }
    return Uri();
  }();
  return ReportDetail(job.m_id, job.m_definition.m_name,
    format_report_parameters(job.m_definition, job.m_parameters),
    std::move(file), status);
}
