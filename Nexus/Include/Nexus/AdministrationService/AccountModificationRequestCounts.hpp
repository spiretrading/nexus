#ifndef NEXUS_ACCOUNT_MODIFICATION_REQUEST_COUNTS_HPP
#define NEXUS_ACCOUNT_MODIFICATION_REQUEST_COUNTS_HPP
#include <format>
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

template<>
class std::formatter<Nexus::AccountModificationRequestCounts> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::AccountModificationRequestCounts& value,
        auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {} {})", value.m_pending,
          value.m_granted, value.m_rejected), context);
    }
};

namespace Nexus {
  inline std::ostream& operator <<(
      std::ostream& out, const AccountModificationRequestCounts& value) {
    return out << std::format(out.getloc(), "{}", value);
  }
}

#endif
