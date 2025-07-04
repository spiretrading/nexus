#ifndef NEXUS_ACCOUNT_ROLES_HPP
#define NEXUS_ACCOUNT_ROLES_HPP
#include <ostream>
#include <Beam/Collections/Enum.hpp>

namespace Nexus::AdministrationService {

  /** Enumerates the roles of a Nexus account has. */
  BEAM_ENUM(AccountRole,

    /** The account has permission to trade. */
    TRADER,

    /** The account is a manager. */
    MANAGER,

    /** The account provides an API service. */
    SERVICE,

    /** The account is an administrator. */
    ADMINISTRATOR);

  using AccountRoles = Beam::EnumSet<AccountRole>;

  inline std::ostream& operator <<(std::ostream& out, AccountRole role) {
    if(role == AccountRole::TRADER) {
      return out << "TRADER";
    } else if(role == AccountRole::MANAGER) {
      return out << "MANAGER";
    } else if(role == AccountRole::SERVICE) {
      return out << "SERVICE";
    } else if(role == AccountRole::ADMINISTRATOR) {
      return out << "ADMINISTRATOR";
    }
    return out << "NONE";
  }

  inline std::ostream& operator <<(std::ostream& out, AccountRole::Type role) {
    return out << AccountRole(role);
  }
}

#endif
