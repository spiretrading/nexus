#ifndef NEXUS_ACCOUNT_MODIFICATION_REQUEST_SUMMARY_HPP
#define NEXUS_ACCOUNT_MODIFICATION_REQUEST_SUMMARY_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <Beam/Serialization/ShuttleVariant.hpp>
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/RiskModification.hpp"

namespace Nexus {

  /** The modification requested by an AccountModificationRequest. */
  using Modification =
    boost::variant<EntitlementModification, RiskModification>;

  /** Stores the details needed to display an AccountModificationRequest. */
  struct AccountModificationRequestSummary {

    /** The request being summarized. */
    AccountModificationRequest m_request;

    /** The request's current status. */
    AccountModificationRequest::Update m_status;

    /** The number of Messages attached to the request. */
    int m_comment_count;

    /** The state prior to the request being applied. */
    boost::optional<Modification> m_previous_state;

    /** The state requested by the modification. */
    boost::optional<Modification> m_modification;

    bool operator ==(const AccountModificationRequestSummary&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const AccountModificationRequestSummary& summary) {
    return out << '(' << summary.m_request << ' ' << summary.m_status << ' ' <<
      summary.m_comment_count << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::AccountModificationRequestSummary> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::AccountModificationRequestSummary& value,
        unsigned int version) const {
      shuttle.shuttle("request", value.m_request);
      shuttle.shuttle("status", value.m_status);
      shuttle.shuttle("comment_count", value.m_comment_count);
      shuttle.shuttle("previous_state", value.m_previous_state);
      shuttle.shuttle("modification", value.m_modification);
    }
  };
}

#endif
