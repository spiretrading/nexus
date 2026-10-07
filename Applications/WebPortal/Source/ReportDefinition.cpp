#include "WebPortal/ReportDefinition.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <unordered_set>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  std::string parse_name(const YAML::Node& node, const std::string& name) {
    auto value = extract<std::string>(node, name);
    if(value.empty()) {
      throw std::runtime_error("Empty report definition field: " + name);
    }
    return value;
  }

  std::vector<std::string> parse_strings(
      const YAML::Node& node, const std::string& name) {
    auto entries = get_node(node, name);
    if(!entries.IsSequence()) {
      throw std::runtime_error("Report field must be a sequence: " + name);
    }
    auto values = std::vector<std::string>();
    for(auto entry : entries) {
      if(!entry.IsScalar()) {
        throw std::runtime_error("Report field requires strings: " + name);
      }
      values.push_back(entry.as<std::string>());
    }
    return values;
  }

  JsonValue parse_json(
      const YAML::Node& node, std::vector<YAML::Node>& ancestors) {
    auto is_circular = std::ranges::any_of(ancestors,
      [&] (const auto& value) {
        return node.is(value);
      });
    if(is_circular) {
      throw std::runtime_error("Circular report parameter default.");
    }
    if(node.IsNull()) {
      return JsonNull();
    } else if(node.IsScalar()) {
      if(node.Tag() == "!" || node.Tag() == "tag:yaml.org,2002:str") {
        return node.Scalar();
      }
      auto boolean = bool();
      if(YAML::convert<bool>::decode(node, boolean)) {
        return boolean;
      }
      auto integer = std::int64_t();
      if(YAML::convert<std::int64_t>::decode(node, integer)) {
        return integer;
      }
      auto decimal = double();
      if(YAML::convert<double>::decode(node, decimal)) {
        if(!std::isfinite(decimal)) {
          throw std::runtime_error("Non-finite report parameter default.");
        }
        return decimal;
      }
      return node.Scalar();
    }
    ancestors.push_back(node);
    auto value = JsonValue();
    if(node.IsSequence()) {
      auto entries = std::vector<JsonValue>();
      for(auto entry : node) {
        entries.push_back(parse_json(entry, ancestors));
      }
      value = std::move(entries);
    } else if(node.IsMap()) {
      auto entries = JsonObject();
      for(auto entry : node) {
        auto name = entry.first.as<std::string>();
        if(entries.get(name)) {
          throw std::runtime_error("Duplicate report default member: " + name);
        }
        entries.set(name, parse_json(entry.second, ancestors));
      }
      value = std::move(entries);
    } else {
      throw std::runtime_error("Invalid report parameter default.");
    }
    ancestors.pop_back();
    return value;
  }
}

ReportDefinition Nexus::parse_report_definition(const YAML::Node& node) {
  auto definition = ReportDefinition();
  definition.m_id = parse_name(node, "id");
  definition.m_name = parse_name(node, "name");
  definition.m_description = extract<std::string>(node, "description", "");
  definition.m_access = parse_strings(node, "access");
  auto parameters = get_node(node, "parameters");
  if(!parameters.IsSequence()) {
    throw std::runtime_error("Report parameters must be a sequence.");
  }
  auto names = std::unordered_set<std::string>();
  for(auto entry : parameters) {
    auto parameter = ReportParameterDefinition();
    parameter.m_name = parse_name(entry, "name");
    if(!names.insert(parameter.m_name).second) {
      throw std::runtime_error(
        "Duplicate report parameter: " + parameter.m_name);
    }
    parameter.m_label = parse_name(entry, "label");
    parameter.m_type = parse_name(entry, "type");
    auto types = std::vector<std::string_view>({"DirectoryEntry",
      "DirectoryEntryList", "Scope", "DateRange", "Date", "DateTime", "Time",
      "Decimal", "Integer", "Money", "Currency"});
    if(!std::ranges::contains(types, parameter.m_type)) {
      throw std::runtime_error(
        "Unsupported report parameter type: " + parameter.m_type);
    }
    parameter.m_is_required = extract<bool>(entry, "required", false);
    if(auto value = entry["default"]) {
      if(parameter.m_type == "Money" && !value.IsNull()) {
        if(auto money = try_parse_money(value.as<std::string>())) {
          parameter.m_default =
            static_cast<Quantity>(*money).get_representation();
        } else {
          throw std::runtime_error("Invalid Money default.");
        }
      } else {
        auto ancestors = std::vector<YAML::Node>();
        parameter.m_default = parse_json(value, ancestors);
      }
    }
    definition.m_parameters.push_back(std::move(parameter));
  }
  definition.m_command = parse_name(node, "command");
  auto arguments = get_node(node, "arguments");
  if(!arguments.IsSequence()) {
    throw std::runtime_error("Report arguments must be a sequence.");
  }
  for(auto entry : arguments) {
    if(entry.IsScalar()) {
      definition.m_arguments.emplace_back(entry.as<std::string>());
    } else {
      auto arguments = OptionalReportArguments();
      arguments.m_if_present = parse_name(entry, "if_present");
      arguments.m_arguments = parse_strings(entry, "arguments");
      definition.m_arguments.emplace_back(std::move(arguments));
    }
  }
  auto output = get_node(node, "output");
  definition.m_output.m_media_type = parse_name(output, "media_type");
  definition.m_output.m_extension = parse_name(output, "extension");
  if(definition.m_output.m_extension.front() == '.' ||
      definition.m_output.m_extension.find_first_of("/\\") !=
        std::string::npos) {
    throw std::runtime_error("Invalid report output extension.");
  }
  return definition;
}

ReportDefinition Nexus::load_report_definition(std::istream& source) {
  return parse_report_definition(YAML::Load(source));
}

std::vector<ReportDefinition> Nexus::filter_report_definitions(
    std::span<const ReportDefinition> definitions,
    const DirectoryEntry& account, ServiceLocatorClient& client) {
  auto current_account = client.load_directory_entry(account.m_id);
  auto names = std::unordered_set<std::string>({current_account.m_name, "*"});
  auto groups_required = std::ranges::any_of(definitions,
    [&] (const auto& definition) {
      return !definition.m_access.empty() &&
        std::ranges::none_of(definition.m_access,
          [&] (const auto& name) { return names.contains(name); });
    });
  if(groups_required) {
    auto pending = std::vector<DirectoryEntry>({current_account});
    auto visited = std::unordered_set<unsigned int>({current_account.m_id});
    while(!pending.empty()) {
      auto entry = pending.back();
      pending.pop_back();
      for(auto& parent : client.load_parents(entry)) {
        if(visited.insert(parent.m_id).second) {
          names.insert(parent.m_name);
          pending.push_back(parent);
        }
      }
    }
  }
  auto permitted = std::vector<ReportDefinition>();
  for(auto& definition : definitions) {
    auto is_permitted = std::ranges::any_of(definition.m_access,
      [&] (const auto& name) { return names.contains(name); });
    if(is_permitted) {
      permitted.push_back(definition);
    }
  }
  return permitted;
}
