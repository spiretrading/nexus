#ifndef NEXUS_COMPLIANCE_PARAMETER_HPP
#define NEXUS_COMPLIANCE_PARAMETER_HPP
#include <string>
#include <type_traits>
#include <variant>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVariant.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Scope.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  class ComplianceValue;

namespace Details {
  using ComplianceVariant = std::variant<bool, Quantity, double, std::string,
    boost::posix_time::ptime, boost::posix_time::time_duration,
    Beam::DirectoryEntry, CurrencyId, Money, Ticker, Venue, Scope,
    std::vector<ComplianceValue>>;
}

  /** Defines the set of types that can be used as a compliance parameter. */
  class ComplianceValue : public Details::ComplianceVariant {
    public:
      using Details::ComplianceVariant::ComplianceVariant;
      using Details::ComplianceVariant::operator =;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const ComplianceValue& value) {
    std::visit([&] (const auto& value) {
      if constexpr(std::is_same_v<std::decay_t<decltype(value)>,
          std::vector<ComplianceValue>>) {
        out << Beam::Stream(value);
      } else {
        out << value;
      }
    }, value);
    return out;
  }

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

namespace Beam {
  template<>
  class Send<Nexus::ComplianceValue> :
    public Send<Nexus::Details::ComplianceVariant> {};

  template<>
  class Receive<Nexus::ComplianceValue> :
    public Receive<Nexus::Details::ComplianceVariant> {};

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
