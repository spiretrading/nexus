#include "WebPortal/ReportActivity.hpp"
#include <algorithm>
#include <compare>
#include <ranges>
#include <string_view>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Utilities/ToString.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Scope.hpp"
#include "Nexus/Definitions/StandardCurrencies.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  std::string join(const std::vector<std::string>& values) {
    return values | std::views::join_with(std::string_view(", ")) |
      std::ranges::to<std::string>();
  }

  std::string format(const std::string& type, const JsonValue& value) {
    if(type == "DirectoryEntry") {
      return get<std::string>(get<JsonObject>(value).at("name"));
    } else if(type == "DirectoryEntryList") {
      auto names = std::vector<std::string>();
      for(auto& entry : get<std::vector<JsonValue>>(value)) {
        names.push_back(format("DirectoryEntry", entry));
      }
      return join(names);
    } else if(type == "Currency") {
      return to_string(CURRENCIES.from(from_json<CurrencyId>(value)).m_code);
    } else if(type == "Money") {
      return to_string(from_json<Money>(value));
    } else if(type == "Date") {
      return to_iso_extended_string(from_undelimited_string(
        get<std::string>(value)));
    } else if(type == "DateTime") {
      return to_iso_extended_string(from_iso_string(get<std::string>(value)));
    } else if(type == "DateRange") {
      auto& range = get<JsonObject>(value);
      auto start = range.get("start");
      auto end = range.get("end");
      auto result = std::string();
      if(start && !get<JsonNull>(&*start)) {
        result = format("Date", *start);
      }
      result += " - ";
      if(end && !get<JsonNull>(&*end)) {
        result += format("Date", *end);
      }
      return result;
    } else if(type == "Scope") {
      auto scope = from_json<Scope>(value);
      if(scope.is_global()) {
        return "*";
      } else if(!scope.get_name().empty()) {
        return scope.get_name();
      }
      auto names = std::vector<std::string>();
      for(auto country : scope.get_countries()) {
        names.push_back(to_string(country));
      }
      for(auto venue : scope.get_venues()) {
        names.push_back(to_string(venue));
      }
      for(auto& ticker : scope.get_tickers()) {
        names.push_back(to_string(ticker));
      }
      std::ranges::sort(names);
      return join(names);
    } else if(auto text = get<std::string>(&value)) {
      return *text;
    }
    return to_string(value);
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
  auto entries = std::vector<std::pair<ReportActivity, ptime>>();
  for(auto& job : jobs) {
    if(job.m_account != account ||
        job.m_account.m_type != DirectoryEntry::Type::ACCOUNT ||
        (job.m_status != ReportJob::Status::QUEUED &&
          job.m_status != ReportJob::Status::RUNNING &&
          job.m_status != ReportJob::Status::FAILED)) {
      continue;
    }
    auto activity = ReportActivity();
    activity.m_id = job.m_id;
    activity.m_type = job.m_definition.m_name;
    for(auto& parameter : job.m_definition.m_parameters) {
      auto value = job.m_parameters.get(parameter.m_name);
      if(value && !get<JsonNull>(&*value)) {
        activity.m_parameters.push_back(format(parameter.m_type, *value));
      }
    }
    if(job.m_status == ReportJob::Status::FAILED) {
      activity.m_status = ReportActivity::Status::FAILED;
    }
    activity.m_date_modified = [&] {
      if(job.m_modified && !job.m_modified->is_special()) {
        return job.m_modified->date();
      }
      if(!job.m_completed.is_special()) {
        return job.m_completed.date();
      }
      return job.m_created.date();
    }();
    entries.emplace_back(std::move(activity), job.m_created);
  }
  std::ranges::sort(entries, [&] (const auto& left, const auto& right) {
    auto& first = left.first;
    auto& second = right.first;
    auto comparison = std::strong_ordering::equal;
    if(query.m_order != ReportActivityQuery::Order::NONE) {
      if(query.m_column == ReportActivityQuery::Column::TYPE) {
        comparison = first.m_type <=> second.m_type;
      } else if(query.m_column == ReportActivityQuery::Column::PARAMETERS) {
        comparison = first.m_parameters <=> second.m_parameters;
      } else if(query.m_column == ReportActivityQuery::Column::STATUS) {
        comparison = first.m_status <=> second.m_status;
      } else if(first.m_date_modified < second.m_date_modified) {
        comparison = std::strong_ordering::less;
      } else if(first.m_date_modified > second.m_date_modified) {
        comparison = std::strong_ordering::greater;
      }
      if(comparison != 0) {
        if(query.m_order == ReportActivityQuery::Order::ASCENDING) {
          return comparison < 0;
        }
        return comparison > 0;
      }
    }
    if(left.second != right.second) {
      return left.second > right.second;
    }
    return first.m_id < second.m_id;
  });
  auto result = ReportActivities();
  result.m_total_count = entries.size();
  if(entries.empty() || query.m_page_index >
      (entries.size() - 1) / ReportActivityQuery::PAGE_SIZE) {
    return result;
  }
  auto start = std::size_t(query.m_page_index) * ReportActivityQuery::PAGE_SIZE;
  auto end = std::min(start + ReportActivityQuery::PAGE_SIZE, entries.size());
  for(auto i = start; i != end; ++i) {
    result.m_activities.push_back(std::move(entries[i].first));
  }
  return result;
}
