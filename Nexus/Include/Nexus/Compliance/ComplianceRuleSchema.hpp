#ifndef NEXUS_COMPLIANCE_RULE_SCHEMA_HPP
#define NEXUS_COMPLIANCE_RULE_SCHEMA_HPP
#include <string>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Compliance/ComplianceParameter.hpp"

namespace Nexus {

  /** Provides a description for a single compliance rule. */
  class ComplianceRuleSchema {
    public:

      /** Constructs an empty ComplianceRuleSchema. */
      ComplianceRuleSchema() = default;

      /**
       * Constructs a ComplianceRuleSchema.
       * @param name The name of the rule.
       * @param parameters The list of parameters.
       */
      ComplianceRuleSchema(
        std::string name, std::vector<ComplianceParameter> parameters) noexcept;

      /** Returns the name of the rule. */
      const std::string& get_name() const;

      /** Returns the rule's parameters. */
      const std::vector<ComplianceParameter>& get_parameters() const;

      bool operator ==(const ComplianceRuleSchema&) const = default;

    private:
      friend struct Beam::Shuttle<ComplianceRuleSchema>;
      std::string m_name;
      std::vector<ComplianceParameter> m_parameters;
  };

  /**
   * Wraps a ComplianceRuleSchema for use by a higher-order
   * ComplianceRuleSchema.
   * @param name The name of the higher order ComplianceRuleSchema.
   * @param parameters The parameters used by the higher-order
   *        ComplianceRuleSchema.
   * @param schema The ComplianceRuleSchema to wrap.
   * @return A higher-order ComplianceRuleSchema.
   */
  inline ComplianceRuleSchema wrap(
      std::string name, std::vector<ComplianceParameter> parameters,
      const ComplianceRuleSchema& schema) {
    parameters.emplace_back("name", schema.get_name());
    auto arguments = std::vector<ComplianceValue>();
    for(auto& parameter : schema.get_parameters()) {
      arguments.push_back(
        std::vector<ComplianceValue>{parameter.m_name, parameter.m_value});
    }
    parameters.emplace_back("arguments", std::move(arguments));
    return ComplianceRuleSchema(std::move(name), std::move(parameters));
  }

  /**
   * Wraps a ComplianceRuleSchema for use by a higher-order
   * ComplianceRuleSchema.
   * @param name The name of the higher order ComplianceRuleSchema.
   * @param schema The ComplianceRuleSchema to wrap.
   * @return A higher-order ComplianceRuleSchema.
   */
  inline ComplianceRuleSchema wrap(
      std::string name, const ComplianceRuleSchema& schema) {
    return wrap(std::move(name), {}, schema);
  }

  /**
   * Unwraps a higher-order ComplianceRuleSchema, returning the wrapped
   * schema.
   * @param schema The higher-order ComplianceRuleSchema to unwrap.
   * @return The ComplianceRuleSchema that was wrapped.
   */
  inline ComplianceRuleSchema unwrap(const ComplianceRuleSchema& schema) {
    auto name = std::string();
    auto arguments = std::vector<ComplianceValue>();
    for(auto& parameter : schema.get_parameters()) {
      if(parameter.m_name == "name") {
        name = boost::get<std::string>(parameter.m_value);
      } else if(parameter.m_name == "arguments") {
        arguments = boost::get<std::vector<ComplianceValue>>(parameter.m_value);
      }
    }
    auto parameters = std::vector<ComplianceParameter>();
    for(auto& argument : arguments) {
      auto& parameter = boost::get<std::vector<ComplianceValue>>(argument);
      if(parameter.size() != 2) {
        boost::throw_with_location(
          std::runtime_error("Invalid ComplianceParameter specified."));
      }
      parameters.push_back(ComplianceParameter(
        boost::get<std::string>(parameter[0]), parameter[1]));
    }
    return ComplianceRuleSchema(std::move(name), std::move(parameters));
  }

  inline std::ostream& operator <<(
      std::ostream& out, const ComplianceRuleSchema& rule) {
    return out << '(' << rule.get_name() << ' ' <<
      Beam::Stream(rule.get_parameters()) << ')';
  }

  inline ComplianceRuleSchema::ComplianceRuleSchema(
    std::string name, std::vector<ComplianceParameter> parameters) noexcept
    : m_name(std::move(name)),
      m_parameters(std::move(parameters)) {}

  inline const std::string& ComplianceRuleSchema::get_name() const {
    return m_name;
  }

  inline const std::vector<ComplianceParameter>&
      ComplianceRuleSchema::get_parameters() const {
    return m_parameters;
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ComplianceRuleSchema> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::ComplianceRuleSchema& value,
        unsigned int version) const {
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("parameters", value.m_parameters);
    }
  };
}

#endif
