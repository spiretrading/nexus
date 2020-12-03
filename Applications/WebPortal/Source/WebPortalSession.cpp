#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace Beam::WebServices;
using namespace Nexus;
using namespace Nexus::WebPortal;

WebPortalSession::WebPortalSession(std::string id)
  : AuthenticatedSession(std::move(id)) {}

ServiceClientsBox& WebPortalSession::GetServiceClients() {
  return *m_serviceClients;
}

void WebPortalSession::SetServiceClients(ServiceClientsBox serviceClients) {
  m_serviceClients.emplace(std::move(serviceClients));
}
