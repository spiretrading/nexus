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
#include <Beam/Utilities/ToString.hpp>
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

  /** Stores a single parameter used by a compliance rule. */
  struct ComplianceParameter {

    /** The name of the parameter. */
    std::string m_name;

    /** The parameter's value. */
    ComplianceValue m_value;

    bool operator ==(const ComplianceParameter&) const = default;
  };
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

template<>
class std::formatter<Nexus::ComplianceValue> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::ComplianceValue& value, auto& context) const ->
        decltype(context.out()) {
      auto text = std::visit([&] (const auto& value) -> std::string {
        if constexpr(std::is_same_v<std::remove_cvref_t<decltype(value)>,
            std::vector<Nexus::ComplianceValue>>) {
          return std::format(context.locale(), "{::}", value);
        } else if constexpr(std::formattable<decltype(value), char>) {
          return std::format(context.locale(), "{}", value);
        } else {
          return Beam::to_string(value);
        }
      }, value);
      return std::formatter<std::string_view>::format(text, context);
    }
};

template<>
class std::formatter<Nexus::ComplianceParameter> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::ComplianceParameter& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {})", value.m_name, value.m_value),
        context);
    }
};

namespace Nexus {
  inline std::ostream& operator <<(
      std::ostream& out, const ComplianceValue& value) {
    auto locale = out.getloc();
    if(auto database = static_cast<const CurrencyDatabase*>(
        out.pword(Beam::ScopedStreamManipulator<CurrencyDatabase>::ID))) {
      locale = make_locale(locale, *database);
    }
    if(auto database = static_cast<const VenueDatabase*>(
        out.pword(Beam::ScopedStreamManipulator<VenueDatabase>::ID))) {
      locale = make_locale(locale, *database);
    }
    return out << std::format(locale, "{}", value);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const ComplianceParameter& value) {
    auto locale = out.getloc();
    if(auto database = static_cast<const CurrencyDatabase*>(
        out.pword(Beam::ScopedStreamManipulator<CurrencyDatabase>::ID))) {
      locale = make_locale(locale, *database);
    }
    if(auto database = static_cast<const VenueDatabase*>(
        out.pword(Beam::ScopedStreamManipulator<VenueDatabase>::ID))) {
      locale = make_locale(locale, *database);
    }
    return out << std::format(locale, "{}", value);
  }
}

#endif
