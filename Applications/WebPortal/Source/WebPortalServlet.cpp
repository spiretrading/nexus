#include "WebPortal/WebPortalServlet.hpp"
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Nexus/OrderExecutionService/StandardQueries.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

WebPortalServlet::WebPortalServlet(
  ServiceLocatorWebServlet::ClientsBuilder clients_builder, Clients clients)
  : m_file_store("web_app"),
    m_service_locator_servlet(Ref(m_sessions), std::move(clients_builder)),
    m_definitions_servlet(Ref(m_sessions)),
    m_administration_servlet(Ref(m_sessions)),
    m_market_data_servlet(Ref(m_sessions)),
    m_compliance_servlet(Ref(m_sessions)),
    m_risk_servlet(Ref(m_sessions), std::move(clients)) {}

WebPortalServlet::~WebPortalServlet() {
  close();
}

std::vector<HttpRequestSlot> WebPortalServlet::get_slots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(matches_path(HttpMethod::GET, "/"),
    std::bind_front(&WebPortalServlet::on_index, this));
  slots.emplace_back(matches_path(HttpMethod::GET, ""),
    std::bind_front(&WebPortalServlet::on_index, this));
  slots.emplace_back(matches_path(HttpMethod::GET, "/index.html"),
    std::bind_front(&WebPortalServlet::on_index, this));
  slots.emplace_back(match_any(HttpMethod::GET),
    std::bind_front(&WebPortalServlet::on_serve_file, this));
  auto service_locator_slots = m_service_locator_servlet.get_slots();
  slots.insert(
    slots.end(), service_locator_slots.begin(), service_locator_slots.end());
  auto definitions_slots = m_definitions_servlet.get_slots();
  slots.insert(slots.end(), definitions_slots.begin(), definitions_slots.end());
  auto administration_slots = m_administration_servlet.get_slots();
  slots.insert(
    slots.end(), administration_slots.begin(), administration_slots.end());
  auto market_data_slots = m_market_data_servlet.get_slots();
  slots.insert(slots.end(), market_data_slots.begin(), market_data_slots.end());
  auto compliance_slots = m_compliance_servlet.get_slots();
  slots.insert(slots.end(), compliance_slots.begin(), compliance_slots.end());
  auto risk_slots = m_risk_servlet.get_slots();
  slots.insert(slots.end(), risk_slots.begin(), risk_slots.end());
  return slots;
}

std::vector<HttpUpgradeSlot<WebPortalServlet::WebSocketChannel>>
    WebPortalServlet::get_web_socket_slots() {
  auto slots = std::vector<HttpUpgradeSlot<WebSocketChannel>>();
  auto risk_slots = m_risk_servlet.get_web_socket_slots();
  slots.insert(slots.end(), risk_slots.begin(), risk_slots.end());
  return slots;
}

void WebPortalServlet::close() {
  if(m_open_state.set_closing()) {
    return;
  }
  m_risk_servlet.close();
  m_compliance_servlet.close();
  m_market_data_servlet.close();
  m_administration_servlet.close();
  m_definitions_servlet.close();
  m_service_locator_servlet.close();
  m_open_state.close();
}

HttpResponse WebPortalServlet::on_index(const HttpRequest& request) {
  auto response = HttpResponse();
  m_file_store.serve("index.html", out(response));
  return response;
}

HttpResponse WebPortalServlet::on_serve_file(const HttpRequest& request) {
  auto response = m_file_store.serve(request);
  if(response.get_status_code() == HttpStatusCode::NOT_FOUND) {
    return on_index(request);
  }
  return response;
}
