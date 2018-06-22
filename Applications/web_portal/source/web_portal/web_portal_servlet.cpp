#include "web_portal/web_portal/web_portal_servlet.hpp"
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
using namespace std;

WebPortalServlet::WebPortalServlet(
    RefType<ApplicationServiceClients> serviceClients)
    : m_fileStore{"web_app"},
      m_serviceClients{serviceClients.Get()},
      m_serviceLocatorServlet{Ref(m_sessions), Ref(*m_serviceClients)},
      m_definitionsServlet{Ref(m_sessions), Ref(*m_serviceClients)},
      m_administrationServlet{Ref(m_sessions), Ref(*m_serviceClients)},
      m_marketDataServlet{Ref(m_sessions), Ref(*m_serviceClients)},
      m_complianceServlet{Ref(m_sessions), Ref(*m_serviceClients)},
      m_riskServlet{Ref(m_sessions), Ref(*m_serviceClients)} {}

WebPortalServlet::~WebPortalServlet() {
  Close();
}

vector<HttpRequestSlot> WebPortalServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  slots.emplace_back(MatchesPath(HttpMethod::GET, "/"),
    std::bind(&WebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, ""),
    std::bind(&WebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, "/index.html"),
    std::bind(&WebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchAny(HttpMethod::GET),
    std::bind(&WebPortalServlet::OnServeFile, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/reporting_service/load_profit_and_loss_report"),
    std::bind(&WebPortalServlet::OnLoadProfitAndLossReport, this,
    std::placeholders::_1));
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

vector<HttpUpgradeSlot<WebPortalServlet::WebSocketChannel>>
    WebPortalServlet::GetWebSocketSlots() {
  vector<HttpUpgradeSlot<WebSocketChannel>> slots;
  auto riskSlots = m_riskServlet.GetWebSocketSlots();
  slots.insert(slots.end(), riskSlots.begin(), riskSlots.end());
  return slots;
}

void WebPortalServlet::Open() {
  if(m_openState.SetOpening()) {
    return;
  }
  try {
    m_serviceClients->Open();
    m_serviceLocatorServlet.Open();
    m_definitionsServlet.Open();
    m_administrationServlet.Open();
    m_marketDataServlet.Open();
    m_complianceServlet.Open();
    m_riskServlet.Open();
  } catch(const std::exception&) {
    m_openState.SetOpenFailure();
    Shutdown();
  }
  m_openState.SetOpen();
}

void WebPortalServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void WebPortalServlet::Shutdown() {
  m_riskServlet.Close();
  m_complianceServlet.Close();
  m_marketDataServlet.Close();
  m_administrationServlet.Close();
  m_definitionsServlet.Close();
  m_serviceLocatorServlet.Close();
  m_serviceClients->Close();
  m_openState.SetClosed();
}

HttpResponse WebPortalServlet::OnIndex(const HttpRequest& request) {
  HttpResponse response;
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

HttpResponse WebPortalServlet::OnLoadProfitAndLossReport(
    const HttpRequest& request) {
  struct Parameters {
    DirectoryEntry m_directoryEntry;
    ptime m_startDate;
    ptime m_endDate;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("directory_entry", m_directoryEntry);
      shuttle.Shuttle("start_date", m_startDate);
      shuttle.Shuttle("end_date", m_endDate);
    }
  };
  HttpResponse response;
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    response.SetStatusCode(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->ShuttleParameters<Parameters>(request);
  auto orderQueue = std::make_shared<Queue<const Order*>>();
  auto marketDatabase =
    m_serviceClients->GetDefinitionsClient().LoadMarketDatabase();
  auto timeZoneDatabase =
    m_serviceClients->GetDefinitionsClient().LoadTimeZoneDatabase();
  QueryDailyOrderSubmissions(parameters.m_directoryEntry,
    parameters.m_startDate, parameters.m_endDate, marketDatabase,
    timeZoneDatabase, m_serviceClients->GetOrderExecutionClient(), orderQueue);
  WebPortfolio portfolio{marketDatabase};
  try {
    while(true) {
      auto order = orderQueue->Top();
      orderQueue->Pop();
      vector<ExecutionReport> executionReports;
      order->GetPublisher().WithSnapshot(
        [&] (auto snapshot) {
          if(snapshot.is_initialized()) {
            executionReports = *snapshot;
          }
        });
      for(auto& executionReport : executionReports) {
        portfolio.Update(order->GetInfo().m_fields, executionReport);
      }
    }
  } catch(const PipeBrokenException&) {}
  vector<WebInventory> inventories;
  for(auto& inventory : portfolio.GetBookkeeper().GetInventoryRange()) {
    inventories.push_back(inventory.second);
  }
  session->ShuttleResponse(inventories, Store(response));
  return response;
}
