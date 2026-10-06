#include "WebPortal/ReportJob.hpp"
#include <algorithm>
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
      return to_iso_extended_string(
        from_undelimited_string(get<std::string>(value)));
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

std::string Nexus::format_report_parameter(
    const ReportParameterDefinition& parameter, const JsonValue& value) {
  return format(parameter.m_type, value);
}

std::vector<std::string> Nexus::format_report_parameters(const ReportJob& job) {
  auto result = std::vector<std::string>();
  for(auto& parameter : job.m_definition.m_parameters) {
    auto value = job.m_parameters.get(parameter.m_name);
    if(value && !get<JsonNull>(&*value)) {
      result.push_back(format_report_parameter(parameter, *value));
    }
  }
  return result;
}
