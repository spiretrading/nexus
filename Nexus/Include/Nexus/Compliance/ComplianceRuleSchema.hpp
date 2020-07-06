#ifndef NEXUS_COMPLIANCE_RULE_SCHEMA_HPP
#define NEXUS_COMPLIANCE_RULE_SCHEMA_HPP
#include <string>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceParameter.hpp"

namespace Nexus::Compliance {

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
      ComplianceRuleSchema(std::string name,
        std::vector<ComplianceParameter> parameters);

      /** Returns the name of the rule. */
      const std::string& GetName() const;

      /** Returns the rule's parameters. */
      const std::vector<ComplianceParameter>& GetParameters() const;

    private:
      friend struct Beam::Serialization::Shuttle<ComplianceRuleSchema>;
      std::string m_name;
      std::vector<ComplianceParameter> m_parameters;
  };

  /**
   * Tests if two ComplianceRuleSchema's are equal.
   * @param lhs The left hand side of the comparison.
   * @param rhs The right hand side of the comparison.
   * @return <code>true</code> iff the two ComplianceRuleSchema's are equal.
   */
  inline bool operator ==(const ComplianceRuleSchema& lhs,
      const ComplianceRuleSchema& rhs) {
    return lhs.GetName() == rhs.GetName() && lhs.GetParameters() ==
      rhs.GetParameters();
  }

  /**
   * Tests if two ComplianceRuleSchema's are not equal.
   * @param lhs The left hand side of the comparison.
   * @param rhs The right hand side of the comparison.
   * @return <code>true</code> iff the two ComplianceRuleSchema's are not equal.
   */
  inline bool operator !=(const ComplianceRuleSchema& lhs,
      const ComplianceRuleSchema& rhs) {
    return !(lhs == rhs);
  }

  inline ComplianceRuleSchema::ComplianceRuleSchema(std::string name,
    std::vector<ComplianceParameter> parameters)
    : m_name(std::move(name)),
      m_parameters(std::move(parameters)) {}

  inline const std::string& ComplianceRuleSchema::GetName() const {
    return m_name;
  }

  inline const std::vector<ComplianceParameter>& ComplianceRuleSchema::
      GetParameters() const {
    return m_parameters;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::Compliance::ComplianceRuleSchema> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::Compliance::ComplianceRuleSchema& value, unsigned int version) {
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("parameters", value.m_parameters);
    }
  };
}

#endif
