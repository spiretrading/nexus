#include "WebPortal/ReportFile.hpp"

using namespace Nexus;

std::filesystem::path Nexus::make_report_filename(const ReportJob& job) {
  auto name = job.m_definition.m_id + '-' + job.m_id + '.' +
    job.m_definition.m_output.m_extension;
  return std::filesystem::path(std::u8string(name.begin(), name.end()));
}
