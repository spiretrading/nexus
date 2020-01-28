#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace Beam::WebServices;
using namespace Nexus;
using namespace Nexus::WebPortal;

WebPortalSession::WebPortalSession(std::string id)
  : AuthenticatedSession(std::move(id)) {}

VirtualServiceClients& WebPortalSession::GetServiceClients() {
  return *m_serviceClients;
}

void WebPortalSession::SetServiceClients(
    std::unique_ptr<VirtualServiceClients> serviceClients) {
  m_serviceClients = std::move(serviceClients);
}
