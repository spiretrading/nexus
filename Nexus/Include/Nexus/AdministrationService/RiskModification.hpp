#ifndef NEXUS_ADMINISTRATION_SERVICE_RISK_MODIFICATION_HPP
#define NEXUS_ADMINISTRATION_SERVICE_RISK_MODIFICATION_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus {

  /** Stores a request to update an account's risk parameters. */
  class RiskModification {
    public:

      /** Constructs an empty RiskModification. */
      RiskModification() = default;

      /**
       * Constructs a RiskModification.
       * @param parameters The risk parameters being requested.
       */
      RiskModification(RiskParameters parameters) noexcept;

      /** Returns the requested risk parameters. */
      const RiskParameters& get_parameters() const;

    private:
      friend struct Beam::Shuttle<RiskModification>;
      RiskParameters m_parameters;
  };

  inline RiskModification::RiskModification(RiskParameters parameters) noexcept
    : m_parameters(std::move(parameters)) {}

  inline const RiskParameters& RiskModification::get_parameters() const {
    return m_parameters;
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::RiskModification> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::RiskModification& value,
        unsigned int version) const {
      shuttle.shuttle("parameters", value.m_parameters);
    }
  };
}

#endif
