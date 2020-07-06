#ifndef NEXUS_COMPLIANCE_PARAMETER_HPP
#define NEXUS_COMPLIANCE_PARAMETER_HPP
#include <string>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVariant.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"

namespace Nexus::Compliance {

  /** Defines the set of types that can be used as a compliance parameter. */
  using ComplianceValue = boost::make_recursive_variant<bool, Quantity, double,
    std::string, boost::posix_time::ptime, boost::posix_time::time_duration,
    CurrencyId, Money, Security, std::vector<boost::recursive_variant_>>::type;

  /** Stores a single parameter used by a compliance rule. */
  struct ComplianceParameter {

    /** The name of the parameter. */
    std::string m_name;

    /** The parameter's value. */
    ComplianceValue m_value;

    /** Constructs an empty parameter. */
    ComplianceParameter() = default;

    /**
     * Constructs a ComplianceParameter.
     * @param name The parameter's name.
     * @param value The parameter's value.
     */
    ComplianceParameter(std::string name, ComplianceValue value);
  };

  /**
   * Tests if two ComplianceParameter's are equal.
   * @param lhs The left hand side of the comparison.
   * @param rhs The right hand side of the comparison.
   * @return <code>true</code> iff the two ComplianceParameter's are equal.
   */
  inline bool operator ==(const ComplianceParameter& lhs,
      const ComplianceParameter& rhs) {
    return lhs.m_name == rhs.m_name && lhs.m_value == rhs.m_value;
  }

  /**
   * Tests if two ComplianceParameter's are not equal.
   * @param lhs The left hand side of the comparison.
   * @param rhs The right hand side of the comparison.
   * @return <code>true</code> iff the two ComplianceParameter's are not equal.
   */
  inline bool operator !=(const ComplianceParameter& lhs,
      const ComplianceParameter& rhs) {
    return !(lhs == rhs);
  }

  inline ComplianceParameter::ComplianceParameter(std::string name,
    ComplianceValue value)
    : m_name(std::move(name)),
      m_value(std::move(value)) {}
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::Compliance::ComplianceParameter> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::Compliance::ComplianceParameter& value, unsigned int version) {
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("value", value.m_value);
    }
  };
}

#endif
