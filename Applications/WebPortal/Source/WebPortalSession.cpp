#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace Beam::WebServices;
using namespace Nexus;

WebPortalSession::WebPortalSession(std::string id)
  : AuthenticatedSession(std::move(id)) {}

Clients& WebPortalSession::get_clients() {
  return *m_clients;
}

void WebPortalSession::set_clients(Clients clients) {
  m_clients.emplace(std::move(clients));
}
