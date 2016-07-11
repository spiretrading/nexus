#ifndef NEXUS_ACCOUNTROLES_HPP
#define NEXUS_ACCOUNTROLES_HPP
#include <Beam/Collections/Enum.hpp>
#include "Nexus/AdministrationService/AdministrationService.hpp"

namespace Nexus {
namespace AdministrationService {

  /*! \enum AccountRole
      \brief Enumerates the roles of a Nexus account has.
   */
  BEAM_ENUM(AccountRole,

    //! The account has permission to trade.
    TRADER,

    //! The account is a manager.
    MANAGER,

    //! The account provides an API service.
    SERVICE,

    //! The account is an administrator.
    ADMINISTRATOR);

  using AccountRoles = Beam::EnumSet<AccountRole>;
}
}

#endif
