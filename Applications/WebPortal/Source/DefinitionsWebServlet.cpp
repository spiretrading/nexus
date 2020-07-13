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
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_organization_name"),
    std::bind(&DefinitionsWebServlet::OnLoadOrganizationName, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_compliance_rule_schemas"),
    std::bind(&DefinitionsWebServlet::OnLoadComplianceRuleSchemas, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_country_database"),
    std::bind(&DefinitionsWebServlet::OnLoadCountryDatabase, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_currency_database"),
    std::bind(&DefinitionsWebServlet::OnLoadCurrencyDatabase, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_destination_database"),
    std::bind(&DefinitionsWebServlet::OnLoadDestinationDatabase, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_exchange_rates"),
    std::bind(&DefinitionsWebServlet::OnLoadExchangeRates, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/definitions_service/load_market_database"),
    std::bind(&DefinitionsWebServlet::OnLoadMarketDatabase, this,
    std::placeholders::_1));
  return slots;
}

void DefinitionsWebServlet::Open() {
  if(m_openState.SetOpening()) {
    return;
  }
  m_openState.SetOpen();
}

void DefinitionsWebServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void DefinitionsWebServlet::Shutdown() {
  m_openState.SetClosed();
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
