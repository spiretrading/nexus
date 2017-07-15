#include "ClientWebPortal/ClientWebPortal/RiskWebServlet.hpp"
#include <Beam/Stomp/StompServer.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "ClientWebPortal/ClientWebPortal/ClientWebPortalSession.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Stomp;
using namespace Beam::Threading;
using namespace Beam::WebServices;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::ClientWebPortal;
using namespace Nexus::RiskService;
using namespace std;

RiskWebServlet::RiskWebServlet(
    RefType<SessionStore<ClientWebPortalSession>> sessions,
    RefType<ApplicationServiceClients> serviceClients)
    : m_sessions{sessions.Get()},
      m_serviceClients{serviceClients.Get()},
      m_portfolioModel{Ref(*m_serviceClients)} {}

RiskWebServlet::~RiskWebServlet() {
  Close();
}

vector<HttpRequestSlot> RiskWebServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  return slots;
}

vector<HttpUpgradeSlot<RiskWebServlet::WebSocketChannel>>
    RiskWebServlet::GetWebSocketSlots() {
  vector<HttpUpgradeSlot<WebSocketChannel>> slots;
  slots.emplace_back(MatchesPath(HttpMethod::GET,
    "/api/risk_service/portfolio"), std::bind(
    &RiskWebServlet::OnPortfolioUpgrade, this, std::placeholders::_1,
    std::placeholders::_2));
  return slots;
}

void RiskWebServlet::Open() {
  if(m_openState.SetOpening()) {
    return;
  }
  try {
    m_serviceClients->Open();
    m_portfolioTimer = m_serviceClients->BuildTimer(seconds(1));
    m_portfolioTimer->GetPublisher().Monitor(m_tasks.GetSlot<Timer::Result>(
      std::bind(&RiskWebServlet::OnPortfolioTimerExpired, this,
      std::placeholders::_1)));
    m_portfolioModel.GetPublisher().Monitor(
      m_tasks.GetSlot<PortfolioModel::Entry>(
      std::bind(&RiskWebServlet::OnPortfolioUpdate, this,
      std::placeholders::_1)));
    m_portfolioModel.Open();
    m_portfolioTimer->Start();
  } catch(const std::exception&) {
    m_openState.SetOpenFailure();
    Shutdown();
  }
  m_openState.SetOpen();
}

void RiskWebServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void RiskWebServlet::Shutdown() {
  m_openState.SetClosed();
}

void RiskWebServlet::OnPortfolioUpgrade(const HttpRequest& request,
    std::unique_ptr<WebSocketChannel> channel) {
  auto session = m_sessions->Find(request);
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
        if(frame.GetCommand() == StompCommand::SEND) {
        } else if(frame.GetCommand() == StompCommand::SUBSCRIBE) {
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

void RiskWebServlet::OnPortfolioUpdate(
    const PortfolioModel::Entry& entry) {
  m_updatedPortfolioEntries.insert(entry);
}

void RiskWebServlet::OnPortfolioTimerExpired(Timer::Result result) {
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
