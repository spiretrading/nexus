#include "WebPortal/ReportSubmission.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <regex>
#include <stdexcept>
#include <unordered_set>
#include <Beam/Json/JsonParser.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Scope.hpp"
#include "Nexus/Definitions/StandardCurrencies.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  const JsonValue& read_member(
      const JsonObject& object, const std::string& name) {
    if(auto value = object.get(name)) {
      return *value;
    }
    throw std::invalid_argument("Missing member: " + name);
  }

  const JsonObject& read_object(const JsonValue& value) {
    if(auto object = get<JsonObject>(&value)) {
      return *object;
    }
    throw std::invalid_argument("Expected an object.");
  }

  const std::string& read_string(const JsonValue& value) {
    if(auto text = get<std::string>(&value)) {
      if(text->find('\0') == std::string::npos) {
        return *text;
      }
    }
    throw std::invalid_argument("Expected a string without null bytes.");
  }

  double read_number(const JsonValue& value) {
    if(auto number = get<double>(&value)) {
      if(std::isfinite(*number)) {
        return *number;
      }
    }
    throw std::invalid_argument("Expected a finite number.");
  }

  double read_integer(const JsonValue& value, double minimum, double maximum) {
    auto number = read_number(value);
    if(std::trunc(number) != number || number < minimum || number > maximum) {
      throw std::invalid_argument("Expected an integer in range.");
    }
    return number;
  }

  JsonValue encode_value(const auto& value) {
    return parse<JsonValue>(to_json(value));
  }

  JsonValue read_date(const JsonValue& value) {
    if(get<JsonNull>(&value)) {
      return JsonNull();
    }
    auto text = read_string(value);
    static const auto PATTERN =
      std::regex("[0-9]{8}|[0-9]{4}-[0-9]{2}-[0-9]{2}");
    if(!std::regex_match(text, PATTERN)) {
      throw std::invalid_argument("Invalid date.");
    }
    std::erase(text, '-');
    return to_iso_string(from_undelimited_string(text));
  }

  time_duration read_time(const std::string& text) {
    constexpr auto HOURS_PER_DAY = 24;
    constexpr auto MINUTES_PER_HOUR = 60;
    constexpr auto SECONDS_PER_MINUTE = 60;
    static const auto PATTERN =
      std::regex("([0-9]{2}):([0-9]{2}):([0-9]{2})(\\.[0-9]{1,6})?");
    auto match = std::smatch();
    if(!std::regex_match(text, match, PATTERN) ||
        std::stoi(match[1]) >= HOURS_PER_DAY ||
        std::stoi(match[2]) >= MINUTES_PER_HOUR ||
        std::stoi(match[3]) >= SECONDS_PER_MINUTE) {
      throw std::invalid_argument("Invalid time of day.");
    }
    return duration_from_string(text);
  }

  JsonValue normalize(const ReportParameterDefinition& parameter,
      const JsonValue& value, const DirectoryEntry& account,
      ServiceLocatorClient& client) {
    auto& type = parameter.m_type;
    if(type == "DirectoryEntry") {
      return encode_value(resolve_report_entry(value, account, client, true));
    } else if(type == "DirectoryEntryList") {
      auto entries = std::vector<JsonValue>();
      auto identifiers = std::unordered_set<unsigned int>();
      for(auto& value : get<std::vector<JsonValue>>(value)) {
        auto entry = resolve_report_entry(value, account, client, true);
        if(!identifiers.insert(entry.m_id).second) {
          throw std::invalid_argument("Duplicate directory entry.");
        }
        entries.push_back(encode_value(entry));
      }
      if(entries.empty()) {
        return JsonNull();
      }
      return entries;
    } else if(type == "Integer") {
      constexpr auto MAXIMUM = static_cast<double>(
        (std::uint64_t(1) << std::numeric_limits<double>::digits) - 1);
      return read_integer(value, -MAXIMUM, MAXIMUM);
    } else if(type == "Decimal") {
      return read_number(value);
    } else if(type == "Money") {
      if(auto text = get<std::string>(&value)) {
        return encode_value(parse_money(*text));
      }
      constexpr auto MAXIMUM = static_cast<double>(
        (std::uint64_t(1) << std::numeric_limits<double>::digits) - 1);
      return read_integer(value, -MAXIMUM, MAXIMUM);
    } else if(type == "Currency") {
      auto currency = [&] {
        if(auto text = get<std::string>(&value)) {
          return parse_currency(*text);
        }
        return CurrencyId(static_cast<std::uint16_t>(read_integer(
          value, 1, std::numeric_limits<std::uint16_t>::max() - 1)));
      }();
      if(!currency || CURRENCIES.from(currency).m_id == CurrencyId::NONE) {
        throw std::invalid_argument("Invalid currency.");
      }
      return static_cast<int>(static_cast<std::uint16_t>(currency));
    } else if(type == "Date") {
      return read_date(value);
    } else if(type == "Time") {
      return to_simple_string(read_time(read_string(value)));
    } else if(type == "DateTime") {
      return to_iso_string(parse_report_datetime(value));
    } else if(type == "DateRange") {
      auto& object = read_object(value);
      auto range = JsonObject();
      auto start = object.get("start");
      auto end = object.get("end");
      range["start"] = JsonNull();
      range["end"] = JsonNull();
      if(start) {
        range["start"] = read_date(*start);
      }
      if(end) {
        range["end"] = read_date(*end);
      }
      auto first = get<std::string>(&range.at("start"));
      auto last = get<std::string>(&range.at("end"));
      if(!first && !last) {
        return JsonNull();
      } else if(parameter.m_is_required && (!first || !last)) {
        throw std::invalid_argument("Both date range bounds are required.");
      } else if(first && last && *first > *last) {
        throw std::invalid_argument("Date range bounds are reversed.");
      }
      return range;
    } else if(type == "Scope") {
      if(value == JsonValue("*")) {
        return encode_value(Scope::GLOBAL);
      }
      auto& object = read_object(value);
      read_string(object.at("name"));
      get<bool>(object.at("is_global"));
      for(auto& country : get<std::vector<JsonValue>>(object.at("countries"))) {
        read_integer(country, 1, 999);
      }
      for(auto& venue : get<std::vector<JsonValue>>(object.at("venues"))) {
        auto& text = read_string(venue);
        if(text.empty() || text.size() > 4) {
          throw std::invalid_argument("Invalid venue.");
        }
      }
      for(auto& ticker : get<std::vector<JsonValue>>(object.at("tickers"))) {
        auto& entry = read_object(ticker);
        if(read_string(entry.at("symbol")).empty() ||
            std::ranges::any_of(read_string(entry.at("symbol")),
              [] (auto character) {
                return std::isspace(static_cast<unsigned char>(character));
              }) ||
            read_string(entry.at("venue")).empty() ||
            read_string(entry.at("venue")).size() > 4) {
          throw std::invalid_argument("Invalid ticker.");
        }
      }
      auto scope = from_json<Scope>(value);
      if(scope.is_empty()) {
        return JsonNull();
      }
      return encode_value(scope);
    }
    throw std::invalid_argument("Unsupported report parameter type.");
  }

  const JsonValue* resolve(const ReportDefinition& definition,
      const JsonObject& parameters, const std::string& path) {
    auto separator = path.find('.');
    auto name = path.substr(0, separator);
    auto parameter = std::ranges::find(
      definition.m_parameters, name, &ReportParameterDefinition::m_name);
    if(parameter == definition.m_parameters.end()) {
      throw std::runtime_error("Unknown report argument parameter: " + name);
    }
    auto value = parameters.get(name);
    if(!value || get<JsonNull>(&*value)) {
      return nullptr;
    }
    if(parameter->m_type == "Currency" && path == name + ".code") {
      return &*value;
    }
    while(separator != std::string::npos) {
      auto begin = separator + 1;
      separator = path.find('.', begin);
      auto& object = read_object(*value);
      value = object.get(path.substr(begin, separator - begin));
      if(!value || get<JsonNull>(&*value)) {
        return nullptr;
      }
    }
    return &*value;
  }

  std::string expand(const ReportDefinition& definition,
      const JsonObject& parameters, const std::string& argument) {
    auto result = std::string();
    for(auto i = std::size_t(0); i < argument.size();) {
      if(argument[i] == '{') {
        auto end = argument.find('}', i + 1);
        if(end == std::string::npos) {
          throw std::runtime_error("Unclosed report argument placeholder.");
        }
        auto path = argument.substr(i + 1, end - i - 1);
        auto value = resolve(definition, parameters, path);
        if(!value) {
          throw std::invalid_argument("Missing report argument: " + path);
        }
        if(auto text = get<std::string>(value)) {
          result += *text;
        } else {
          result += to_string(*value);
        }
        i = end + 1;
      } else if(argument[i] == '}') {
        throw std::runtime_error("Unexpected report argument brace.");
      } else {
        result += argument[i];
        ++i;
      }
    }
    if(result.find('\0') != std::string::npos) {
      throw std::invalid_argument("Null byte in report argument.");
    }
    return result;
  }
}

ReportNotFoundException::ReportNotFoundException()
  : std::runtime_error("Report not found.") {}

DirectoryEntry Nexus::parse_report_entry(const JsonValue& value) {
  auto& object = read_object(value);
  auto type = static_cast<int>(read_integer(read_member(object, "type"), 0, 1));
  auto id = static_cast<unsigned int>(read_integer(read_member(object, "id"), 0,
    std::numeric_limits<unsigned int>::max()));
  return DirectoryEntry(DirectoryEntry::Type(type), id, "");
}

ptime Nexus::parse_report_datetime(const JsonValue& value) {
  auto text = read_string(value);
  static const auto PATTERN = std::regex(
    "([0-9]{8}T[0-9]{6}|[0-9]{4}-[0-9]{2}-[0-9]{2}T"
    "[0-9]{2}:[0-9]{2}:[0-9]{2})(\\.[0-9]{1,6})?");
  if(!std::regex_match(text, PATTERN)) {
    throw std::invalid_argument("Invalid date and time.");
  }
  std::erase(text, '-');
  std::erase(text, ':');
  auto day = read_date(text.substr(0, 8));
  auto time = read_time(text.substr(9, 2) + ":" + text.substr(11, 2) + ":" +
    text.substr(13));
  return ptime(from_undelimited_string(get<std::string>(day)), time);
}

ReportSchedule::Interval Nexus::parse_report_interval(const JsonValue& value) {
  auto& object = read_object(value);
  auto count = read_integer(
    read_member(object, "count"), 1, std::numeric_limits<std::uint32_t>::max());
  auto unit = read_integer(read_member(object, "unit"), 0,
    static_cast<int>(ReportSchedule::Interval::Unit::YEAR));
  return ReportSchedule::Interval(static_cast<std::uint32_t>(count),
    ReportSchedule::Interval::Unit(static_cast<int>(unit)));
}

ReportSchedule Nexus::prepare_report_schedule(const ReportSchedule& schedule,
    const ReportScheduleSubmission& submission,
    const std::vector<ReportDefinition>& definitions,
    ServiceLocatorClient& client, ptime now) {
  auto& interval = submission.m_repeat_interval;
  if(interval && (interval->m_count == 0 ||
      interval->m_unit < ReportSchedule::Interval::Unit::DAY ||
      interval->m_unit > ReportSchedule::Interval::Unit::YEAR)) {
    throw std::invalid_argument("Invalid repeat interval.");
  }
  auto start = convert_report_time(
    schedule.m_start_time, schedule.m_time_zone, submission.m_time_zone);
  auto is_same_interval = [&] {
    if(!interval || !schedule.m_repeat_interval) {
      return interval.has_value() == schedule.m_repeat_interval.has_value();
    }
    return interval->m_count == schedule.m_repeat_interval->m_count &&
      interval->m_unit == schedule.m_repeat_interval->m_unit;
  }();
  auto job = prepare_report_job(
    definitions, schedule.m_account, submission.m_report, client);
  if(start == submission.m_start_time && is_same_interval) {
    return ReportSchedule(schedule.m_id, schedule.m_account,
      std::move(job.m_definition), std::move(job.m_parameters),
      std::move(job.m_recipients), schedule.m_created, schedule.m_start_time,
      schedule.m_run_time, schedule.m_repeat_interval, schedule.m_time_zone);
  }
  auto utc =
    convert_report_time(submission.m_start_time, submission.m_time_zone, "UTC");
  if(!interval && utc <= now) {
    throw std::invalid_argument(
      "A changed one-time start must be in the future.");
  }
  auto result = ReportSchedule(schedule.m_id, schedule.m_account,
    std::move(job.m_definition), std::move(job.m_parameters),
    std::move(job.m_recipients), schedule.m_created, submission.m_start_time,
    submission.m_start_time, interval, submission.m_time_zone);
  result.m_run_time = next_report_run(result, now);
  return result;
}

ReportJob Nexus::prepare_report_job(
    const std::vector<ReportDefinition>& definitions,
    const DirectoryEntry& account, const ReportSubmission& submission,
    ServiceLocatorClient& client) {
  auto permitted = filter_report_definitions(definitions, account, client);
  auto definition = std::ranges::find(
    permitted, submission.m_report_type, &ReportDefinition::m_id);
  if(definition == permitted.end()) {
    throw ReportNotFoundException();
  }
  auto owner = client.load_directory_entry(account.m_id);
  auto parameters = prepare_report_parameters(
    *definition, submission.m_parameters, owner, client);
  auto arguments = make_report_arguments(*definition, parameters);
  auto recipients =
    prepare_report_recipients(submission.m_recipients, owner, client);
  return ReportJob({}, std::move(owner), std::move(recipients), *definition,
    std::move(parameters), std::move(arguments));
}

std::vector<DirectoryEntry> Nexus::prepare_report_recipients(
    const std::vector<DirectoryEntry>& recipients,
    const DirectoryEntry& account, ServiceLocatorClient& client) {
  auto result = std::vector<DirectoryEntry>();
  auto identifiers = std::unordered_set<unsigned int>();
  for(auto& recipient : recipients) {
    auto entry =
      resolve_report_entry(encode_value(recipient), account, client, false);
    if(identifiers.insert(entry.m_id).second) {
      result.push_back(std::move(entry));
    }
  }
  return result;
}

void Nexus::validate_report_retries(const std::vector<ReportJob>& jobs,
    const std::vector<ReportDefinition>& definitions,
    ServiceLocatorClient& client) {
  for(auto& job : jobs) {
    if(!std::ranges::contains(
        definitions, job.m_definition.m_id, &ReportDefinition::m_id)) {
      throw ReportNotFoundException();
    }
    for(auto& parameter : job.m_definition.m_parameters) {
      auto value = job.m_parameters.get(parameter.m_name);
      if(!value || get<JsonNull>(&*value)) {
        continue;
      }
      if(parameter.m_type == "DirectoryEntry") {
        resolve_report_entry(*value, job.m_account, client, true);
      } else if(parameter.m_type == "DirectoryEntryList") {
        for(auto& entry : get<std::vector<JsonValue>>(*value)) {
          resolve_report_entry(entry, job.m_account, client, true);
        }
      }
    }
    for(auto& recipient : job.m_recipients) {
      resolve_report_entry(
        encode_value(recipient), job.m_account, client, false);
    }
  }
}

JsonObject Nexus::prepare_report_parameters(const ReportDefinition& definition,
    const JsonObject& parameters, const DirectoryEntry& account,
    ServiceLocatorClient& client) {
  auto result = JsonObject();
  for(auto& parameter : definition.m_parameters) {
    auto value = parameters.get(parameter.m_name);
    if(!value || get<JsonNull>(&*value)) {
      if(parameter.m_default) {
        value = *parameter.m_default;
      }
    }
    if(value && !get<JsonNull>(&*value)) {
      try {
        result[parameter.m_name] =
          normalize(parameter, *value, account, client);
      } catch(const std::exception&) {
        std::throw_with_nested(std::invalid_argument(
          "Invalid report parameter: " + parameter.m_name));
      }
    }
    auto normalized = result.get(parameter.m_name);
    if(parameter.m_is_required &&
        (!normalized || get<JsonNull>(&*normalized))) {
      throw std::invalid_argument(
        "Required report parameter: " + parameter.m_name);
    }
  }
  return result;
}

DirectoryEntry Nexus::resolve_report_entry(const JsonValue& value,
    const DirectoryEntry& account, ServiceLocatorClient& client,
    bool allow_global) {
  auto requested = parse_report_entry(value);
  if(requested.m_id == DirectoryEntry::STAR_DIRECTORY.m_id) {
    if(allow_global && requested.m_type == DirectoryEntry::Type::DIRECTORY) {
      return DirectoryEntry::STAR_DIRECTORY;
    }
    throw std::invalid_argument("Global sharing is not permitted.");
  }
  auto entry = [&] {
    try {
      return client.load_directory_entry(requested.m_id);
    } catch(const ServiceRequestException&) {
      std::throw_with_nested(
        std::invalid_argument("Invalid account or group."));
    }
  }();
  if(entry.m_type != requested.m_type ||
      !client.has_permissions(account, entry, Permission::READ)) {
    throw std::invalid_argument("Account or group is not accessible.");
  }
  return entry;
}

std::vector<std::string> Nexus::make_report_arguments(
    const ReportDefinition& definition, const JsonObject& parameters) {
  auto result = std::vector<std::string>();
  for(auto& argument : definition.m_arguments) {
    if(auto text = std::get_if<std::string>(&argument)) {
      result.push_back(expand(definition, parameters, *text));
    } else {
      auto& group = std::get<OptionalReportArguments>(argument);
      if(resolve(definition, parameters, group.m_if_present)) {
        for(auto& text : group.m_arguments) {
          result.push_back(expand(definition, parameters, text));
        }
      }
    }
  }
  return result;
}
