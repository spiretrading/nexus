#include "WebPortal/DefinitionsWebServlet.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace Beam::WebServices;
using namespace boost;
using namespace Nexus;
using namespace Nexus::WebPortal;

DefinitionsWebServlet::DefinitionsWebServlet(
  Ref<SessionStore<WebPortalSession>> sessions)
  : m_sessions(sessions.Get()) {}

DefinitionsWebServlet::~DefinitionsWebServlet() {
  Close();
}

std::vector<HttpRequestSlot> DefinitionsWebServlet::GetSlots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/definitions_service/load_organization_name"),
    std::bind_front(&DefinitionsWebServlet::OnLoadOrganizationName, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/definitions_service/load_compliance_rule_schemas"),
    std::bind_front(&DefinitionsWebServlet::OnLoadComplianceRuleSchemas, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/definitions_service/load_country_database"),
    std::bind_front(&DefinitionsWebServlet::OnLoadCountryDatabase, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/definitions_service/load_currency_database"),
    std::bind_front(&DefinitionsWebServlet::OnLoadCurrencyDatabase, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/definitions_service/load_destination_database"),
    std::bind_front(&DefinitionsWebServlet::OnLoadDestinationDatabase, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/definitions_service/load_exchange_rates"),
    std::bind_front(&DefinitionsWebServlet::OnLoadExchangeRates, this));
  slots.emplace_back(MatchesPath(
    HttpMethod::POST, "/api/definitions_service/load_market_database"),
    std::bind_front(&DefinitionsWebServlet::OnLoadMarketDatabase, this));
  return slots;
}

void DefinitionsWebServlet::Close() {
  m_openState.Close();
}

HttpResponse DefinitionsWebServlet::OnLoadOrganizationName(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& serviceClients = session->GetServiceClients();
  auto organizationName =
    serviceClients.GetDefinitionsClient().LoadOrganizationName();
  session->ShuttleResponse(organizationName, Store(response));
  return response;
}

HttpResponse DefinitionsWebServlet::OnLoadComplianceRuleSchemas(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& serviceClients = session->GetServiceClients();
  auto schemas =
    serviceClients.GetDefinitionsClient().LoadComplianceRuleSchemas();
  session->ShuttleResponse(schemas, Store(response));
  return response;
}

HttpResponse DefinitionsWebServlet::OnLoadCountryDatabase(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& serviceClients = session->GetServiceClients();
  auto database = serviceClients.GetDefinitionsClient().LoadCountryDatabase();
  session->ShuttleResponse(database, Store(response));
  return response;
}

HttpResponse DefinitionsWebServlet::OnLoadCurrencyDatabase(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& serviceClients = session->GetServiceClients();
  auto database = serviceClients.GetDefinitionsClient().LoadCurrencyDatabase();
  session->ShuttleResponse(database, Store(response));
  return response;
}

HttpResponse DefinitionsWebServlet::OnLoadDestinationDatabase(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& serviceClients = session->GetServiceClients();
  auto database =
    serviceClients.GetDefinitionsClient().LoadDestinationDatabase();
  session->ShuttleResponse(database, Store(response));
  return response;
}

HttpResponse DefinitionsWebServlet::OnLoadExchangeRates(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& serviceClients = session->GetServiceClients();
  auto exchangeRates =
    serviceClients.GetDefinitionsClient().LoadExchangeRates();
  session->ShuttleResponse(exchangeRates, Store(response));
  return response;
}

HttpResponse DefinitionsWebServlet::OnLoadMarketDatabase(
    const HttpRequest& request) {
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto& serviceClients = session->GetServiceClients();
  auto database = serviceClients.GetDefinitionsClient().LoadMarketDatabase();
  session->ShuttleResponse(database, Store(response));
  return response;
}
