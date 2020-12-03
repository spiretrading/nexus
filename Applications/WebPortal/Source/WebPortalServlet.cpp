#include "WebPortal/WebPortalServlet.hpp"
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Nexus/OrderExecutionService/StandardQueries.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Stomp;
using namespace Beam::Threading;
using namespace Beam::WebServices;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::AdministrationService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace Nexus::WebPortal;

WebPortalServlet::WebPortalServlet(
  ServiceLocatorWebServlet::ServiceClientsBuilder serviceClientsBuilder,
  ServiceClientsBox serviceClients)
  : m_fileStore("web_app"),
    m_serviceLocatorServlet(Ref(m_sessions), std::move(serviceClientsBuilder)),
    m_definitionsServlet(Ref(m_sessions)),
    m_administrationServlet(Ref(m_sessions)),
    m_marketDataServlet(Ref(m_sessions)),
    m_complianceServlet(Ref(m_sessions)),
    m_riskServlet(Ref(m_sessions), std::move(serviceClients)) {}

WebPortalServlet::~WebPortalServlet() {
  Close();
}

std::vector<HttpRequestSlot> WebPortalServlet::GetSlots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(MatchesPath(HttpMethod::GET, "/"),
    std::bind(&WebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, ""),
    std::bind(&WebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, "/index.html"),
    std::bind(&WebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchAny(HttpMethod::GET),
    std::bind(&WebPortalServlet::OnServeFile, this, std::placeholders::_1));
  auto serviceLocatorSlots = m_serviceLocatorServlet.GetSlots();
  slots.insert(slots.end(), serviceLocatorSlots.begin(),
    serviceLocatorSlots.end());
  auto definitionsSlots = m_definitionsServlet.GetSlots();
  slots.insert(slots.end(), definitionsSlots.begin(), definitionsSlots.end());
  auto administrationSlots = m_administrationServlet.GetSlots();
  slots.insert(slots.end(), administrationSlots.begin(),
    administrationSlots.end());
  auto marketDataSlots = m_marketDataServlet.GetSlots();
  slots.insert(slots.end(), marketDataSlots.begin(), marketDataSlots.end());
  auto complianceSlots = m_complianceServlet.GetSlots();
  slots.insert(slots.end(), complianceSlots.begin(), complianceSlots.end());
  auto riskSlots = m_riskServlet.GetSlots();
  slots.insert(slots.end(), riskSlots.begin(), riskSlots.end());
  return slots;
}

std::vector<HttpUpgradeSlot<WebPortalServlet::WebSocketChannel>>
    WebPortalServlet::GetWebSocketSlots() {
  auto slots = std::vector<HttpUpgradeSlot<WebSocketChannel>>();
  auto riskSlots = m_riskServlet.GetWebSocketSlots();
  slots.insert(slots.end(), riskSlots.begin(), riskSlots.end());
  return slots;
}

void WebPortalServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  m_riskServlet.Close();
  m_complianceServlet.Close();
  m_marketDataServlet.Close();
  m_administrationServlet.Close();
  m_definitionsServlet.Close();
  m_serviceLocatorServlet.Close();
  m_openState.Close();
}

HttpResponse WebPortalServlet::OnIndex(const HttpRequest& request) {
  auto response = HttpResponse();
  m_fileStore.Serve("index.html", Store(response));
  return response;
}

HttpResponse WebPortalServlet::OnServeFile(const HttpRequest& request) {
  auto response = m_fileStore.Serve(request);
  if(response.GetStatusCode() == HttpStatusCode::NOT_FOUND) {
    return OnIndex(request);
  }
  return response;
}
