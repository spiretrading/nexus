#ifndef NEXUS_ADMINISTRATION_SERVICE_ENTITLEMENT_MODIFICATION_REQUEST_HPP
#define NEXUS_ADMINISTRATION_SERVICE_ENTITLEMENT_MODIFICATION_REQUEST_HPP
#include <vector>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/AdministrationService/AdministrationService.hpp"

namespace Nexus {
namespace AdministrationService {

  /*! \class EntitlementModificationRequest
      \brief Stores a request to update an account's market data entitlements.
  */
  class EntitlementModificationRequest {
    public:

      //! Constructs an EntitlementModificationRequest.
      /*!
        \param entitlements The list of entitlements to grant to the account.
      */
      EntitlementModificationRequest(
        std::vector<Beam::ServiceLocator::DirectoryEntry> entitlements);

      //! Returns the list of entitlements to grant.
      const std::vector<Beam::ServiceLocator::DirectoryEntry>&
        GetEntitlements() const;

    private:
      friend struct Beam::Serialization::Shuttle<
        EntitlementModificationRequest>;
      std::vector<Beam::ServiceLocator::DirectoryEntry> m_entitlements;

      EntitlementModificationRequest(Beam::Serialization::ReceiveBuilder);
  };

  inline EntitlementModificationRequest::EntitlementModificationRequest(
      std::vector<Beam::ServiceLocator::DirectoryEntry> entitlements)
      : m_entitlements(std::move(entitlements)) {}

  inline const std::vector<Beam::ServiceLocator::DirectoryEntry>&
      EntitlementModificationRequest::GetEntitlements() const {
    return m_entitlements;
  }
}
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Nexus::AdministrationService::EntitlementModificationRequest> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::AdministrationService::EntitlementModificationRequest& value,
        unsigned int version) {
      shuttle.Shuttle("entitlements", value.m_entitlements);
    }
  };
}
}

#endif
