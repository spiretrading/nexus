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
using namespace Nexus::MarketDataService;
using namespace Nexus::WebPortal;

MarketDataWebServlet::MarketDataWebServlet(
  Ref<SessionStore<WebPortalSession>> sessions)
  : m_sessions(sessions.Get()) {}

MarketDataWebServlet::~MarketDataWebServlet() {
  Close();
}

std::vector<HttpRequestSlot> MarketDataWebServlet::GetSlots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/market_data_service/load_security_info_from_prefix"),
    std::bind_front(&MarketDataWebServlet::OnLoadSecurityInfoFromPrefix, this));
  return slots;
}

void MarketDataWebServlet::Close() {
  m_openState.Close();
}

HttpResponse MarketDataWebServlet::OnLoadSecurityInfoFromPrefix(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_prefix;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("prefix", m_prefix);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto& serviceClients = session->GetServiceClients();
  auto securityInfos =
    serviceClients.GetMarketDataClient().LoadSecurityInfoFromPrefix(
    parameters.m_prefix);
  session->ShuttleResponse(securityInfos, Store(response));
  return response;
}
