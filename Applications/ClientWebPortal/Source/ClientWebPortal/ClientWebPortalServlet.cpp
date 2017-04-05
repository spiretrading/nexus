#include "ClientWebPortal/ClientWebPortal/ClientWebPortalServlet.hpp"
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Stomp/StompServer.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
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
using namespace Nexus::ClientWebPortal;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace std;

namespace {
  using WebPosition = Nexus::Accounting::Position<Nexus::Security>;
  using WebInventory = Nexus::Accounting::Inventory<WebPosition>;
  using WebBookkeeper = Nexus::Accounting::TrueAverageBookkeeper<WebInventory>;
  using WebPortfolio = Nexus::Accounting::Portfolio<WebBookkeeper>;
}

ClientWebPortalServlet::ClientWebPortalServlet(
    RefType<ApplicationServiceClients> serviceClients)
    : m_fileStore{"webapp"},
      m_serviceClients{serviceClients.Get()},
      m_serviceLocatorServlet{Ref(m_sessions), Ref(*m_serviceClients)},
      m_definitionsServlet{Ref(m_sessions), Ref(*m_serviceClients)},
      m_administrationServlet{Ref(m_sessions), Ref(*m_serviceClients)},
      m_marketDataServlet{Ref(m_sessions), Ref(*m_serviceClients)},
      m_complianceServlet{Ref(m_sessions), Ref(*m_serviceClients)},
      m_portfolioModel{Ref(*m_serviceClients)} {}

ClientWebPortalServlet::~ClientWebPortalServlet() {
  Close();
}

vector<HttpRequestSlot> ClientWebPortalServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  slots.emplace_back(MatchesPath(HttpMethod::GET, "/"),
    std::bind(&ClientWebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, ""),
    std::bind(&ClientWebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::GET, "/index.html"),
    std::bind(&ClientWebPortalServlet::OnIndex, this, std::placeholders::_1));
  slots.emplace_back(MatchAny(HttpMethod::GET),
    std::bind(&ClientWebPortalServlet::OnServeFile, this,
    std::placeholders::_1));
  slots.emplace_back(MatchesPath(HttpMethod::POST,
    "/api/reporting_service/load_profit_and_loss_report"),
    std::bind(&ClientWebPortalServlet::OnLoadProfitAndLossReport, this,
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
  return slots;
}

vector<HttpUpgradeSlot<ClientWebPortalServlet::WebSocketChannel>>
    ClientWebPortalServlet::GetWebSocketSlots() {
  vector<HttpUpgradeSlot<WebSocketChannel>> slots;
  slots.emplace_back(MatchesPath(HttpMethod::GET,
    "/api/risk_service/portfolio"), std::bind(
    &ClientWebPortalServlet::OnPortfolioUpgrade, this, std::placeholders::_1,
    std::placeholders::_2));
  return slots;
}

void ClientWebPortalServlet::Open() {
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
    m_portfolioTimer = m_serviceClients->BuildTimer(seconds(1));
    m_portfolioTimer->GetPublisher().Monitor(m_tasks.GetSlot<Timer::Result>(
      std::bind(&ClientWebPortalServlet::OnPortfolioTimerExpired, this,
      std::placeholders::_1)));
    m_portfolioModel.GetPublisher().Monitor(
      m_tasks.GetSlot<PortfolioModel::Entry>(
      std::bind(&ClientWebPortalServlet::OnPortfolioUpdate, this,
      std::placeholders::_1)));
    m_portfolioModel.Open();
    m_portfolioTimer->Start();
  } catch(const std::exception&) {
    m_openState.SetOpenFailure();
    Shutdown();
  }
  m_openState.SetOpen();
}

void ClientWebPortalServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void ClientWebPortalServlet::Shutdown() {
  m_portfolioModel.Close();
  m_complianceServlet.Close();
  m_marketDataServlet.Close();
  m_administrationServlet.Close();
  m_definitionsServlet.Close();
  m_serviceLocatorServlet.Close();
  m_serviceClients->Close();
  m_openState.SetClosed();
}

HttpResponse ClientWebPortalServlet::OnIndex(const HttpRequest& request) {
  HttpResponse response;
  m_fileStore.Serve("index.html", Store(response));
  return response;
}

HttpResponse ClientWebPortalServlet::OnServeFile(const HttpRequest& request) {
  auto response = m_fileStore.Serve(request);
  if(response.GetStatusCode() == HttpStatusCode::NOT_FOUND) {
    return OnIndex(request);
  }
  return response;
}

HttpResponse ClientWebPortalServlet::OnLoadProfitAndLossReport(
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

void ClientWebPortalServlet::OnPortfolioUpgrade(const HttpRequest& request,
    std::unique_ptr<WebSocketChannel> channel) {
  auto session = m_sessions.Find(request);
  if(session == nullptr) {
    channel->GetConnection().Close();
    return;
  }
  auto subscriber = std::make_shared<PortfolioSubscriber>();
  subscriber->m_client = std::make_shared<StompServer>(std::move(channel));
  Routines::Spawn(
    [=] {
      subscriber->m_client->Open();
      while(true) {
        auto frame = subscriber->m_client->Read();
        if(frame.GetCommand() == StompCommand::SUBSCRIBE) {
          auto idHeader = frame.FindHeader("id");
          if(!idHeader.is_initialized()) {
            continue;
          }
          subscriber->m_subscriptionId = *idHeader;
          m_tasks.Push(
            [=] {
              JsonSender<SharedBuffer> sender;
              for(auto& entry : m_portfolioEntries) {
                StompFrame entryFrame{StompCommand::MESSAGE};
                entryFrame.AddHeader(
                  {"subscription", subscriber->m_subscriptionId});
                entryFrame.AddHeader(
                  {"destination", "/api/risk_service/portfolio"});
                entryFrame.AddHeader(
                  {"content-type", "application/json"});
                auto buffer = Encode<SharedBuffer>(sender, entry.second);
                entryFrame.SetBody(std::move(buffer));
                try {
                  subscriber->m_client->Write(entryFrame);
                } catch(const std::exception&) {
                  return;
                }
              }
              m_porfolioSubscribers.push_back(subscriber);
            });
        }
      }
    });
}

void ClientWebPortalServlet::OnPortfolioUpdate(
    const PortfolioModel::Entry& entry) {
  m_updatedPortfolioEntries.insert(entry);
}

void ClientWebPortalServlet::OnPortfolioTimerExpired(Timer::Result result) {
  vector<PortfolioModel::Entry> updatedEntries;
  updatedEntries.reserve(m_updatedPortfolioEntries.size());
  std::move(m_updatedPortfolioEntries.begin(), m_updatedPortfolioEntries.end(),
    std::back_inserter(updatedEntries));
  for(auto& updatedEntry : updatedEntries) {
    RiskPortfolioKey key{updatedEntry.m_account,
      updatedEntry.m_inventory.m_position.m_key.m_index};
    auto entryResult = m_portfolioEntries.insert(make_pair(key, updatedEntry));
    if(!entryResult.second) {
      entryResult.first->second = updatedEntry;
    }
  }
  m_updatedPortfolioEntries.clear();
  JsonSender<SharedBuffer> sender;
  auto i = m_porfolioSubscribers.begin();
  while(i != m_porfolioSubscribers.end()) {
    auto& subscriber = *i;
    try {
      for(auto& entry : updatedEntries) {
        StompFrame entryFrame{StompCommand::MESSAGE};
        entryFrame.AddHeader({"subscription", subscriber->m_subscriptionId});
        entryFrame.AddHeader({"destination", "/api/risk_service/portfolio"});
        entryFrame.AddHeader({"content-type", "application/json"});
        auto buffer = Encode<SharedBuffer>(sender, entry);
        entryFrame.SetBody(std::move(buffer));
        subscriber->m_client->Write(entryFrame);
      }
      ++i;
    } catch(const std::exception&) {
      i = m_porfolioSubscribers.erase(i);
    }
  }
  m_portfolioTimer->Start();
}
