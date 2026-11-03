#ifndef NEXUS_REPORT_DEFINITION_HPP
#define NEXUS_REPORT_DEFINITION_HPP
#include <istream>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>
#include <Beam/Json/JsonObject.hpp>
#include <Beam/Serialization/ShuttleJsonValue.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <Beam/Serialization/ShuttleVariant.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <yaml-cpp/yaml.h>

namespace Nexus {

  /** Describes a report parameter. */
  struct ReportParameterDefinition {

    /** The parameter identifier. */
    std::string m_name;

    /** The displayed label. */
    std::string m_label;

    /** The shared parameter type name. */
    std::string m_type;

    /** Whether the parameter requires a value. */
    bool m_is_required = false;

    /** The default value, or no value when no default is specified. */
    std::optional<Beam::JsonValue> m_default;
  };

  /** Describes the generated report's file format. */
  struct ReportOutputDefinition {

    /** The output's media type. */
    std::string m_media_type;

    /** The file extension without a leading dot. */
    std::string m_extension;

    /**
     * The filename stem template with parameter placeholders such as
     * "report_{period.start}_{period.end}". The extension is added
     * automatically. Empty uses the report id and local creation date.
     */
    std::string m_filename;
  };

  /** Arguments included when a parameter or one of its members is present. */
  struct OptionalReportArguments {

    /** The parameter or member whose presence includes these arguments. */
    std::string m_if_present;

    /** The arguments in command-line order. */
    std::vector<std::string> m_arguments;
  };

  /** A command-line argument or a conditional group of arguments. */
  using ReportArgument = std::variant<std::string, OptionalReportArguments>;

  /** Defines a report's public metadata, access, and executable command. */
  struct ReportDefinition {

    /** The stable report type identifier. */
    std::string m_id;

    /** The displayed report name. */
    std::string m_name;

    /** The report description. */
    std::string m_description;

    /** Permitted account/group names, with "*" granting global access. */
    std::vector<std::string> m_access;

    /** The parameters in display order. */
    std::vector<ReportParameterDefinition> m_parameters;

    /** The executable command. */
    std::string m_command;

    /** The command's arguments in order. */
    std::vector<ReportArgument> m_arguments;

    /** The generated file's format. */
    ReportOutputDefinition m_output;
  };

  /** Parses a report definition from YAML. */
  ReportDefinition parse_report_definition(const YAML::Node& node);

  /**
   * Loads a report definition from a YAML stream.
   * @param source - The stream containing one report definition.
   * @return The report definition.
   */
  ReportDefinition load_report_definition(std::istream& source);

  /**
   * Selects definitions permitted for an account and its ancestor groups.
   * @param definitions - The available report definitions.
   * @param account - The authenticated account.
   * @param client - A service locator client able to read group memberships.
   * @return The permitted definitions in their original order.
   */
  std::vector<ReportDefinition> filter_report_definitions(
    std::span<const ReportDefinition> definitions,
    const Beam::DirectoryEntry& account, Beam::ServiceLocatorClient& client);
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ReportParameterDefinition> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::ReportParameterDefinition& value,
        unsigned int version) const {
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("label", value.m_label);
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("required", value.m_is_required);
      shuttle.shuttle("default", value.m_default);
    }
  };

  template<>
  struct Shuttle<Nexus::ReportOutputDefinition> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::ReportOutputDefinition& value,
        unsigned int version) const {
      shuttle.shuttle("media_type", value.m_media_type);
      shuttle.shuttle("extension", value.m_extension);
      shuttle.shuttle("filename", value.m_filename);
    }
  };

  template<>
  struct Shuttle<Nexus::OptionalReportArguments> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::OptionalReportArguments& value,
        unsigned int version) const {
      shuttle.shuttle("if_present", value.m_if_present);
      shuttle.shuttle("arguments", value.m_arguments);
    }
  };

  template<>
  struct Shuttle<Nexus::ReportDefinition> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::ReportDefinition& value,
        unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("description", value.m_description);
      shuttle.shuttle("access", value.m_access);
      shuttle.shuttle("parameters", value.m_parameters);
      shuttle.shuttle("command", value.m_command);
      shuttle.shuttle("arguments", value.m_arguments);
      shuttle.shuttle("output", value.m_output);
    }
  };
}

#endif
