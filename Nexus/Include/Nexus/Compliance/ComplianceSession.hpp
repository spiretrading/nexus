#ifndef NEXUS_COMPLIANCESESSION_HPP
#define NEXUS_COMPLIANCESESSION_HPP
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>
#include "Nexus/Compliance/Compliance.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class ComplianceSession
      \brief Stores session info for a ComplianceServlet client.
   */
  class ComplianceSession : public Beam::ServiceLocator::AuthenticatedSession {
    public:

      //! Constructs a ComplianceSession.
      ComplianceSession() = default;
  };
}
}

#endif
