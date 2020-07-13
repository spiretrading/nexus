#ifndef NEXUS_ADMINISTRATION_SERVICE_ENTITLEMENT_MODIFICATION_HPP
#define NEXUS_ADMINISTRATION_SERVICE_ENTITLEMENT_MODIFICATION_HPP
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/AdministrationService/AdministrationService.hpp"

namespace Nexus::AdministrationService {

  /** Stores a request to update an account's market data entitlements. */
  class EntitlementModification {
    public:

      /** Constructs an empty EntitlementModification. */
      EntitlementModification() = default;

      /**
       * Constructs an EntitlementModification.
       * @param entitlements The list of entitlements to grant to the account.
       */
      EntitlementModification(
        std::vector<Beam::ServiceLocator::DirectoryEntry> entitlements);

      /** Returns the list of entitlements to grant. */
      const std::vector<Beam::ServiceLocator::DirectoryEntry>&
        GetEntitlements() const;

    private:
      friend struct Beam::Serialization::Shuttle<EntitlementModification>;
      std::vector<Beam::ServiceLocator::DirectoryEntry> m_entitlements;
  };

  inline EntitlementModification::EntitlementModification(
    std::vector<Beam::ServiceLocator::DirectoryEntry> entitlements)
    : m_entitlements(std::move(entitlements)) {}

  inline const std::vector<Beam::ServiceLocator::DirectoryEntry>&
      EntitlementModification::GetEntitlements() const {
    return m_entitlements;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::AdministrationService::EntitlementModification> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::AdministrationService::EntitlementModification& value,
        unsigned int version) {
      shuttle.Shuttle("entitlements", value.m_entitlements);
    }
  };
}

#endif
