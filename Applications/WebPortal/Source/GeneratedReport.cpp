#include "WebPortal/GeneratedReport.hpp"
#include <algorithm>
#include <locale>
#include <sstream>
#include <tuple>
#include <boost/algorithm/string.hpp>
#include "WebPortal/ReportAccess.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  std::string format_date(date value) {
    static const auto LOCALE =
      std::locale(std::locale::classic(), new date_facet("%b %d, %Y"));
    auto stream = std::ostringstream();
    stream.imbue(LOCALE);
    stream << value;
    return stream.str();
  }
}

GeneratedReports Nexus::query_generated_reports(
    const std::vector<ReportJob>& jobs, const DirectoryEntry& account,
    const GeneratedReportQuery& query, ServiceLocatorClient& client) {
  if(account.m_type != DirectoryEntry::Type::ACCOUNT ||
      query.m_column < GeneratedReportQuery::Column::TYPE ||
      query.m_column > GeneratedReportQuery::Column::DATE_CREATED ||
      query.m_order < GeneratedReportQuery::Order::NONE ||
      query.m_order > GeneratedReportQuery::Order::DESCENDING ||
      (query.m_start_date && query.m_start_date->is_special()) ||
      (query.m_end_date && query.m_end_date->is_special()) ||
      (query.m_start_date && query.m_end_date &&
        *query.m_start_date > *query.m_end_date)) {
    throw std::invalid_argument("Invalid generated report query.");
  }
  auto access = ReportAccess(account, client);
  auto text = trim_copy(query.m_query);
  auto result = GeneratedReports();
  auto entries = std::vector<std::pair<GeneratedReport, ptime>>();
  for(auto& job : jobs) {
    if(!access.is_accessible(job)) {
      continue;
    }
    result.m_is_empty = false;
    auto generated = [&] {
      if(!job.m_completed.is_special()) {
        return job.m_completed;
      }
      return job.m_created;
    }();
    auto day = generated.date();
    if((query.m_start_date && day < *query.m_start_date) ||
        (query.m_end_date && day > *query.m_end_date)) {
      continue;
    }
    auto report = GeneratedReport(
      job.m_id, job.m_definition.m_name, format_report_parameters(job),
      Uri("/reports/" + uri_encode(job.m_id)), day);
    auto matches = text.empty() || icontains(report.m_type, text) ||
      icontains(format_date(day), text) ||
      std::ranges::any_of(report.m_parameters, [&] (const auto& parameter) {
        return icontains(parameter, text);
      });
    if(matches) {
      entries.emplace_back(std::move(report), generated);
    }
  }
  result.m_filtered_count = entries.size();
  if(entries.empty() || query.m_page_index >
      (entries.size() - 1) / GeneratedReportQuery::PAGE_SIZE) {
    return result;
  }
  auto start =
    std::size_t(query.m_page_index) * GeneratedReportQuery::PAGE_SIZE;
  auto end = std::min(start + GeneratedReportQuery::PAGE_SIZE, entries.size());
  std::ranges::partial_sort(entries, entries.begin() + end,
      [&] (const auto& left, const auto& right) {
    auto& first = left.first;
    auto& second = right.first;
    if(query.m_order == GeneratedReportQuery::Order::NONE) {
      return std::tie(right.second, first.m_id) <
        std::tie(left.second, second.m_id);
    }
    auto compare = [&] (const auto& first_value, const auto& second_value) {
      if(query.m_order == GeneratedReportQuery::Order::ASCENDING) {
        return std::tie(first_value, right.second, first.m_id) <
          std::tie(second_value, left.second, second.m_id);
      }
      return std::tie(second_value, right.second, first.m_id) <
        std::tie(first_value, left.second, second.m_id);
    };
    if(query.m_column == GeneratedReportQuery::Column::TYPE) {
      return compare(first.m_type, second.m_type);
    } else if(query.m_column == GeneratedReportQuery::Column::PARAMETERS) {
      return compare(first.m_parameters, second.m_parameters);
    }
    return compare(first.m_date_created, second.m_date_created);
  });
  result.m_reports.reserve(end - start);
  for(auto i = start; i != end; ++i) {
    result.m_reports.push_back(std::move(entries[i].first));
  }
  return result;
}
