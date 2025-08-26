#include "WebPortal/MarketDataWebServlet.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::WebServices;
using namespace boost;
using namespace Nexus;

MarketDataWebServlet::MarketDataWebServlet(
  Ref<SessionStore<WebPortalSession>> sessions)
  : m_sessions(sessions.Get()) {}

MarketDataWebServlet::~MarketDataWebServlet() {
  close();
}

std::vector<HttpRequestSlot> MarketDataWebServlet::get_slots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/market_data_service/load_security_info_from_prefix"), std::bind_front(
      &MarketDataWebServlet::on_load_security_info_from_prefix, this));
  return slots;
}

void MarketDataWebServlet::close() {
  m_open_state.Close();
}

HttpResponse MarketDataWebServlet::on_load_security_info_from_prefix(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_prefix;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("prefix", m_prefix);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(!session) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto security_infos =
    clients.get_market_data_client().load_security_info_from_prefix(
      parameters.m_prefix);
  session->shuttle_response(security_infos, Store(response));
  return response;
}
