#ifndef NEXUS_DEFINITIONSSESSION_HPP
#define NEXUS_DEFINITIONSSESSION_HPP
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>
#include "Nexus/DefinitionsService/DefinitionsService.hpp"

namespace Nexus {
namespace DefinitionsService {

  /*! \class DefinitionsSession
      \brief Stores session info for a DefinitionsServlet client.
   */
  class DefinitionsSession : public Beam::ServiceLocator::AuthenticatedSession {
    public:

      //! Constructs a DefinitionsSession.
      DefinitionsSession();
  };

  inline DefinitionsSession::DefinitionsSession() {}
}
}

#endif
