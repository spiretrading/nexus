#ifndef NEXUS_ADMINISTRATIONSESSION_HPP
#define NEXUS_ADMINISTRATIONSESSION_HPP
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>
#include "Nexus/AdministrationService/AdministrationService.hpp"

namespace Nexus {
namespace AdministrationService {

  /*! \class AdministrationSession
      \brief Stores session info for an AdministrationServlet client.
   */
  class AdministrationSession
      : public Beam::ServiceLocator::AuthenticatedSession {
    public:

      //! Constructs an AdministrationSession.
      AdministrationSession();
  };

  inline AdministrationSession::AdministrationSession() {}
}
}

#endif
