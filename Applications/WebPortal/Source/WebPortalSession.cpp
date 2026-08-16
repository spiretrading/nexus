#include "WebPortal/WebPortalSession.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

using namespace Beam;
using namespace Nexus;

WebPortalSession::WebPortalSession(std::string id)
    : AuthenticatedWebSession(std::move(id)),
      m_receiver(Ref(m_types)),
      m_sender(Ref(m_types)) {
  Nexus::register_query_types(out(m_types));
}

Clients& WebPortalSession::get_clients() {
  return *m_clients;
}

void WebPortalSession::set_clients(Clients clients) {
  m_clients.emplace(std::move(clients));
}
