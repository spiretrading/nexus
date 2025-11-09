#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace Nexus;

WebPortalSession::WebPortalSession(std::string id)
  : AuthenticatedWebSession(std::move(id)) {}

Clients& WebPortalSession::get_clients() {
  return *m_clients;
}

void WebPortalSession::set_clients(Clients clients) {
  m_clients.emplace(std::move(clients));
}
