#ifndef NEXUS_ACCOUNT_QUERY_RESULT_HPP
#define NEXUS_ACCOUNT_QUERY_RESULT_HPP
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>

namespace Nexus {

  /** Stores the result of an account query. */
  struct AccountQueryResult {

    /** The account's DirectoryEntry. */
    Beam::DirectoryEntry m_account;

    /** The account's display name. */
    std::string m_name;
  };
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::AccountQueryResult> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::AccountQueryResult& value,
        unsigned int version) const {
      shuttle.shuttle("account", value.m_account);
      shuttle.shuttle("name", value.m_name);
    }
  };
}

#endif
