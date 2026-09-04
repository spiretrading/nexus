#ifndef NEXUS_ACCOUNT_MODIFICATION_REQUEST_COUNTS_HPP
#define NEXUS_ACCOUNT_MODIFICATION_REQUEST_COUNTS_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>

namespace Nexus {

  /** Stores the number of AccountModificationRequests in each state. */
  struct AccountModificationRequestCounts {

    /** The number of requests awaiting a decision. */
    int m_pending = 0;

    /** The number of requests that have been granted. */
    int m_granted = 0;

    /** The number of requests that have been rejected. */
    int m_rejected = 0;

    bool operator ==(const AccountModificationRequestCounts&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const AccountModificationRequestCounts& counts) {
    return out << '(' << counts.m_pending << ' ' << counts.m_granted << ' ' <<
      counts.m_rejected << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::AccountModificationRequestCounts> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::AccountModificationRequestCounts& value,
        unsigned int version) const {
      shuttle.shuttle("pending", value.m_pending);
      shuttle.shuttle("granted", value.m_granted);
      shuttle.shuttle("rejected", value.m_rejected);
    }
  };
}

#endif
