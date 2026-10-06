#include "WebPortal/ScheduledReport.hpp"
#include <algorithm>
#include <locale>
#include <sstream>
#include <tuple>
#include <boost/algorithm/string.hpp>

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
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

ScheduledReports Nexus::query_scheduled_reports(
    const std::vector<ReportSchedule>& schedules, const DirectoryEntry& account,
    const ScheduledReportQuery& query) {
  if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
    throw std::invalid_argument("Invalid scheduled report account.");
  }
  auto result = ScheduledReports();
  auto text = trim_copy(query.m_query);
  auto matches = std::vector<const ReportSchedule*>();
  for(auto& schedule : schedules) {
    if(schedule.m_account != account) {
      continue;
    }
    result.m_is_empty = false;
    auto title = schedule.m_definition.m_name;
    if(schedule.m_repeat_interval) {
      title = "Recurring " + title;
    }
    auto parameters =
      format_report_parameters(schedule.m_definition, schedule.m_parameters);
    auto is_match = text.empty() || icontains(title, text) ||
      icontains(format_date(schedule.m_run_time.date()), text) ||
      std::ranges::any_of(parameters, [&] (const auto& parameter) {
        return icontains(parameter.m_label, text) ||
          icontains(parameter.m_value, text);
      });
    if(is_match) {
      matches.push_back(&schedule);
    }
  }
  std::ranges::sort(matches, [] (const auto* left, const auto* right) {
    return std::tie(right->m_created, left->m_id) <
      std::tie(left->m_created, right->m_id);
  });
  result.m_filtered_count = matches.size();
  if(matches.empty() || query.m_page_index >
      (matches.size() - 1) / ScheduledReportQuery::PAGE_SIZE) {
    return result;
  }
  auto start =
    std::size_t(query.m_page_index) * ScheduledReportQuery::PAGE_SIZE;
  auto end = std::min(start + ScheduledReportQuery::PAGE_SIZE, matches.size());
  for(auto i = start; i != end; ++i) {
    auto& schedule = *matches[i];
    result.m_schedules.emplace_back(schedule.m_id, schedule.m_definition.m_name,
      format_report_parameters(schedule.m_definition, schedule.m_parameters),
      schedule.m_repeat_interval.has_value(), schedule.m_run_time.date());
  }
  return result;
}
