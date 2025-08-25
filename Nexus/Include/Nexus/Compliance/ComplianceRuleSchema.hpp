#ifndef NEXUS_COMPLIANCE_RULE_SCHEMA_HPP
#define NEXUS_COMPLIANCE_RULE_SCHEMA_HPP
#include <string>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Utilities/Streamable.hpp>
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
        std::string name, std::vector<ComplianceParameter> parameters);

      /** Returns the name of the rule. */
      const std::string& get_name() const;

      /** Returns the rule's parameters. */
      const std::vector<ComplianceParameter>& get_parameters() const;

      bool operator ==(const ComplianceRuleSchema&) const = default;

    private:
      friend struct Beam::Serialization::Shuttle<ComplianceRuleSchema>;
      std::string m_name;
      std::vector<ComplianceParameter> m_parameters;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const ComplianceRuleSchema& rule) {
    return out << '(' << rule.get_name() << ' ' <<
      Beam::Stream(rule.get_parameters()) << ')';
  }

  inline ComplianceRuleSchema::ComplianceRuleSchema(
    std::string name, std::vector<ComplianceParameter> parameters)
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

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::ComplianceRuleSchema> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::ComplianceRuleSchema& value,
        unsigned int version) {
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("parameters", value.m_parameters);
    }
  };
}

#endif
