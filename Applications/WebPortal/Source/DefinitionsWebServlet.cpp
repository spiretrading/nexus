#include "WebPortal/DefinitionsWebServlet.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

DefinitionsWebServlet::DefinitionsWebServlet(
  Ref<WebSessionStore<WebPortalSession>> sessions)
  : m_sessions(sessions.get()) {}

DefinitionsWebServlet::~DefinitionsWebServlet() {
  close();
}

std::vector<HttpRequestSlot> DefinitionsWebServlet::get_slots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/definitions_service/load_organization_name"),
    std::bind_front(&DefinitionsWebServlet::on_load_organization_name, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/definitions_service/load_compliance_rule_schemas"),
    std::bind_front(
      &DefinitionsWebServlet::on_load_compliance_rule_schemas, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/definitions_service/load_country_database"),
    std::bind_front(&DefinitionsWebServlet::on_load_country_database, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/definitions_service/load_currency_database"),
    std::bind_front(&DefinitionsWebServlet::on_load_currency_database, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/definitions_service/load_destination_database"),
    std::bind_front(&DefinitionsWebServlet::on_load_destination_database, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/definitions_service/load_exchange_rates"),
    std::bind_front(&DefinitionsWebServlet::on_load_exchange_rates, this));
  slots.emplace_back(matches_path(
    HttpMethod::POST, "/api/definitions_service/load_venue_database"),
    std::bind_front(&DefinitionsWebServlet::on_load_venue_database, this));
  return slots;
}

void DefinitionsWebServlet::close() {
  m_open_state.close();
}

HttpResponse DefinitionsWebServlet::on_load_organization_name(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto organization_name =
    clients.get_definitions_client().load_organization_name();
  session->shuttle_response(organization_name, out(response));
  return response;
}

HttpResponse DefinitionsWebServlet::on_load_compliance_rule_schemas(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto schemas =
    clients.get_definitions_client().load_compliance_rule_schemas();
  session->shuttle_response(schemas, out(response));
  return response;
}

HttpResponse DefinitionsWebServlet::on_load_country_database(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto database = clients.get_definitions_client().load_country_database();
  session->shuttle_response(database, out(response));
  return response;
}

HttpResponse DefinitionsWebServlet::on_load_currency_database(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto database = clients.get_definitions_client().load_currency_database();
  session->shuttle_response(database, out(response));
  return response;
}

HttpResponse DefinitionsWebServlet::on_load_destination_database(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto database = clients.get_definitions_client().load_destination_database();
  session->shuttle_response(database, out(response));
  return response;
}

HttpResponse DefinitionsWebServlet::on_load_exchange_rates(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto exchange_rates = clients.get_definitions_client().load_exchange_rates();
  session->shuttle_response(exchange_rates, out(response));
  return response;
}

HttpResponse DefinitionsWebServlet::on_load_venue_database(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& clients = session->get_clients();
  auto database = clients.get_definitions_client().load_venue_database();
  session->shuttle_response(database, out(response));
  return response;
}
