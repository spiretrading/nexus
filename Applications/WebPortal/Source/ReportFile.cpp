#include "WebPortal/ReportFile.hpp"
#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "WebPortal/ReportSubmission.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace Nexus;

namespace {
  std::string sanitize(std::string name) {
    for(auto& character : name) {
      auto byte = static_cast<unsigned char>(character);
      if(byte < 32 || byte == 127 ||
          std::string_view("<>:\"/\\|?*").contains(character)) {
        character = '_';
      }
    }
    trim(name);
    static constexpr auto MAX_STEM_SIZE = std::size_t(160);
    if(name.size() > MAX_STEM_SIZE) {
      auto size = MAX_STEM_SIZE;
      while((static_cast<unsigned char>(name[size]) & 0xC0) == 0x80) {
        --size;
      }
      name.resize(size);
    }
    while(!name.empty() && (name.back() == '.' || name.back() == ' ')) {
      name.pop_back();
    }
    if(name.empty()) {
      name = "report";
    }
    auto base = name.substr(0, name.find('.'));
    auto reserved = std::ranges::any_of(
      std::initializer_list<std::string_view>{"CON", "PRN", "AUX", "NUL"},
      [&] (auto word) { return iequals(base, word); });
    if(reserved || (base.size() == 4 &&
        (istarts_with(base, "COM") || istarts_with(base, "LPT")) &&
        base.back() >= '1' && base.back() <= '9')) {
      name.insert(name.begin(), '_');
    }
    return name;
  }

  std::string make_stem(const ReportJob& job) {
    if(job.m_definition.m_output.m_filename.empty()) {
      auto created = convert_report_time(job.m_created, "UTC", job.m_time_zone);
      return sanitize(job.m_definition.m_id + '_' +
        to_iso_extended_string(created.date()));
    }
    auto parameters = job.m_parameters;
    for(auto& parameter : job.m_definition.m_parameters) {
      auto value = parameters.get(parameter.m_name);
      if(!value || std::holds_alternative<JsonNull>(*value)) {
        continue;
      }
      if(parameter.m_type == "Date") {
        parameters[parameter.m_name] = to_iso_extended_string(
          from_undelimited_string(get<std::string>(*value)));
      } else if(parameter.m_type == "DateRange") {
        auto range = get<JsonObject>(*value);
        for(auto& bound : {"start", "end"}) {
          auto date = range.get(bound);
          if(date && !std::holds_alternative<JsonNull>(*date)) {
            range[bound] = to_iso_extended_string(
              from_undelimited_string(get<std::string>(*date)));
          }
        }
        parameters[parameter.m_name] = std::move(range);
      }
    }
    return sanitize(expand_report_template(job.m_definition, parameters,
      job.m_definition.m_output.m_filename));
  }
}

std::filesystem::path Nexus::make_report_filename(const ReportJob& job) {
  if(!job.m_filename.empty()) {
    return std::filesystem::path(
      std::u8string(job.m_filename.begin(), job.m_filename.end()));
  }
  return make_report_filename(job, {});
}

std::filesystem::path Nexus::make_report_filename(
    const ReportJob& job, std::span<const std::string> filenames) {
  auto stem = make_stem(job);
  auto extension = '.' + job.m_definition.m_output.m_extension;
  auto name = stem + extension;
  auto suffix = std::size_t(2);
  auto exists = [&] {
    return std::ranges::any_of(filenames,
      [&] (const auto& filename) { return iequals(name, filename); });
  };
  while(exists()) {
    name = stem + '_' + std::to_string(suffix) + extension;
    ++suffix;
  }
  return std::filesystem::path(std::u8string(name.begin(), name.end()));
}
