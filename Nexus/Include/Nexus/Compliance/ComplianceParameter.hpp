#ifndef NEXUS_COMPLIANCE_PARAMETER_HPP
#define NEXUS_COMPLIANCE_PARAMETER_HPP
#include <string>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVariant.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Defines the set of types that can be used as a compliance parameter. */
  using ComplianceValue = boost::make_recursive_variant<bool, Quantity, double,
    std::string, boost::posix_time::ptime, boost::posix_time::time_duration,
    Beam::DirectoryEntry, CurrencyId, Money, Security, Venue, Region,
    std::vector<boost::recursive_variant_>>::type;

  /** Stores a single parameter used by a compliance rule. */
  struct ComplianceParameter {

    /** The name of the parameter. */
    std::string m_name;

    /** The parameter's value. */
    ComplianceValue m_value;

    bool operator ==(const ComplianceParameter&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const ComplianceParameter& parameter) {
    return out << '(' << parameter.m_name << ' ' << parameter.m_value << ')';
  }
}

namespace boost {
  inline std::ostream& operator <<(
      std::ostream& out, const Nexus::ComplianceValue& value) {
    if(auto v = boost::get<std::vector<Nexus::ComplianceValue>>(&value)) {
      return out << Beam::Stream(*v);
    }
    boost::apply_visitor([&] (const auto& value) {
      out << value;
    }, value);
    return out;
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ComplianceParameter> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::ComplianceParameter& value,
        unsigned int version) const {
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("value", value.m_value);
    }
  };
}

#endif
