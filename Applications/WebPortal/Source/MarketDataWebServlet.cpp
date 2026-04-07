#include "WebPortal/MarketDataWebServlet.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

MarketDataWebServlet::MarketDataWebServlet(
  Ref<WebSessionStore<WebPortalSession>> sessions)
  : m_sessions(sessions.get()) {}

MarketDataWebServlet::~MarketDataWebServlet() {
  close();
}

std::vector<HttpRequestSlot> MarketDataWebServlet::get_slots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/market_data_service/load_security_info_from_prefix"), std::bind_front(
      &MarketDataWebServlet::on_load_security_info_from_prefix, this));
  return slots;
}

void MarketDataWebServlet::close() {
  m_open_state.close();
}

HttpResponse MarketDataWebServlet::on_load_security_info_from_prefix(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_prefix;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("prefix", m_prefix);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto security_infos =
    clients.get_market_data_client().load_security_info_from_prefix(
      parameters.m_prefix);
  session->shuttle_response(security_infos, out(response));
  return response;
}
