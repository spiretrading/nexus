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

    bool operator ==(const ComplianceParameter& rhs) const = default;
  };
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
