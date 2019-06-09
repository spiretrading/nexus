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
using namespace std;

MarketDataWebServlet::MarketDataWebServlet(
    Ref<SessionStore<WebPortalSession>> sessions,
    Ref<ApplicationServiceClients> serviceClients)
    : m_sessions{sessions.Get()},
      m_serviceClients{serviceClients.Get()} {}

MarketDataWebServlet::~MarketDataWebServlet() {
  Close();
}

vector<HttpRequestSlot> MarketDataWebServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/market_data_service/load_security_info_from_prefix"),
    std::bind(&MarketDataWebServlet::OnLoadSecurityInfoFromPrefix, this,
    std::placeholders::_1));
  return slots;
}

void MarketDataWebServlet::Open() {
  if(m_openState.SetOpening()) {
    return;
  }
  try {
    m_serviceClients->Open();
  } catch(const std::exception&) {
    m_openState.SetOpenFailure();
    Shutdown();
  }
  m_openState.SetOpen();
}

void MarketDataWebServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void MarketDataWebServlet::Shutdown() {
  m_openState.SetClosed();
}

HttpResponse MarketDataWebServlet::OnLoadSecurityInfoFromPrefix(
    const HttpRequest& request) {
  struct Parameters {
    string m_prefix;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("prefix", m_prefix);
    }
  };
  HttpResponse response;
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto securityInfos =
    m_serviceClients->GetMarketDataClient().LoadSecurityInfoFromPrefix(
    parameters.m_prefix);
  session->ShuttleResponse(securityInfos, Store(response));
  return response;
}
