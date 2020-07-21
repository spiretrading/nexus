#ifndef NEXUS_ADMINISTRATION_SERVICE_RISK_MODIFICATION_HPP
#define NEXUS_ADMINISTRATION_SERVICE_RISK_MODIFICATION_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus::AdministrationService {

  /** Stores a request to update an account's risk parameters. */
  class RiskModification {
    public:

      /** Constructs an empty RiskModification. */
      RiskModification() = default;

      /**
       * Constructs a RiskModification.
       * @param parameters The risk parameters being requested.
       */
      RiskModification(RiskService::RiskParameters parameters);

      /** Returns the requested risk parameters. */
      const RiskService::RiskParameters& GetParameters() const;

    private:
      friend struct Beam::Serialization::Shuttle<RiskModification>;
      RiskService::RiskParameters m_parameters;
  };

  inline RiskModification::RiskModification(
    RiskService::RiskParameters parameters)
    : m_parameters(std::move(parameters)) {}

  inline const RiskService::RiskParameters&
      RiskModification::GetParameters() const {
    return m_parameters;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::AdministrationService::RiskModification> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::AdministrationService::RiskModification& value,
        unsigned int version) {
      shuttle.Shuttle("parameters", value.m_parameters);
    }
  };
}

#endif
