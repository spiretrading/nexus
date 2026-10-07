#include "WebPortal/ReportActivity.hpp"
#include <algorithm>
#include <compare>

using namespace Beam;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  ReportActivity::Status get_status(const ReportJob& job) {
    if(job.m_status == ReportJob::Status::FAILED) {
      return ReportActivity::Status::FAILED;
    }
    return ReportActivity::Status::GENERATING;
  }

  date get_date_modified(const ReportJob& job) {
    if(job.m_modified && !job.m_modified->is_special()) {
      return job.m_modified->date();
    }
    if(!job.m_completed.is_special()) {
      return job.m_completed.date();
    }
    return job.m_created.date();
  }
}

ReportActivities Nexus::query_report_activities(
    const std::vector<ReportJob>& jobs, const DirectoryEntry& account,
    const ReportActivityQuery& query) {
  if(query.m_column < ReportActivityQuery::Column::TYPE ||
      query.m_column > ReportActivityQuery::Column::DATE_MODIFIED ||
      query.m_order < ReportActivityQuery::Order::NONE ||
      query.m_order > ReportActivityQuery::Order::DESCENDING) {
    throw std::invalid_argument("Invalid report activity ordering.");
  }
  struct Entry {
    const ReportJob* m_job;
    date m_date_modified;
    std::vector<std::string> m_parameters;
  };
  auto entries = std::vector<Entry>();
  for(auto& job : jobs) {
    if(job.m_account != account ||
        job.m_account.m_type != DirectoryEntry::Type::ACCOUNT ||
        (job.m_status != ReportJob::Status::QUEUED &&
          job.m_status != ReportJob::Status::RUNNING &&
          job.m_status != ReportJob::Status::FAILED)) {
      continue;
    }
    entries.emplace_back(&job, get_date_modified(job));
  }
  auto result = ReportActivities();
  result.m_total_count = entries.size();
  if(entries.empty() || query.m_page_index >
      (entries.size() - 1) / ReportActivityQuery::PAGE_SIZE) {
    return result;
  }
  auto is_parameter_sort = query.m_order != ReportActivityQuery::Order::NONE &&
    query.m_column == ReportActivityQuery::Column::PARAMETERS;
  if(is_parameter_sort) {
    for(auto& entry : entries) {
      entry.m_parameters = format_report_parameters(*entry.m_job);
    }
  }
  std::ranges::sort(entries, [&] (const auto& left, const auto& right) {
    auto& first = *left.m_job;
    auto& second = *right.m_job;
    auto comparison = std::strong_ordering::equal;
    if(query.m_order != ReportActivityQuery::Order::NONE) {
      if(query.m_column == ReportActivityQuery::Column::TYPE) {
        comparison = first.m_definition.m_name <=> second.m_definition.m_name;
      } else if(query.m_column == ReportActivityQuery::Column::PARAMETERS) {
        comparison = left.m_parameters <=> right.m_parameters;
      } else if(query.m_column == ReportActivityQuery::Column::STATUS) {
        comparison = get_status(first) <=> get_status(second);
      } else if(left.m_date_modified < right.m_date_modified) {
        comparison = std::strong_ordering::less;
      } else if(left.m_date_modified > right.m_date_modified) {
        comparison = std::strong_ordering::greater;
      }
      if(comparison != 0) {
        if(query.m_order == ReportActivityQuery::Order::ASCENDING) {
          return comparison < 0;
        }
        return comparison > 0;
      }
    }
    if(first.m_created != second.m_created) {
      return first.m_created > second.m_created;
    }
    return first.m_id < second.m_id;
  });
  auto start = std::size_t(query.m_page_index) * ReportActivityQuery::PAGE_SIZE;
  auto end = std::min(start + ReportActivityQuery::PAGE_SIZE, entries.size());
  result.m_activities.reserve(end - start);
  for(auto i = start; i != end; ++i) {
    auto& job = *entries[i].m_job;
    auto parameters = std::move(entries[i].m_parameters);
    if(!is_parameter_sort) {
      parameters = format_report_parameters(job);
    }
    result.m_activities.emplace_back(job.m_id, job.m_definition.m_name,
      std::move(parameters), get_status(job), entries[i].m_date_modified);
  }
  return result;
}
