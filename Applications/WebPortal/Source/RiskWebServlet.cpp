#include "WebPortal/RiskWebServlet.hpp"
#include <algorithm>
#include <Beam/Stomp/StompServer.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Stomp;
using namespace Beam::Threading;
using namespace Beam::WebServices;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::RiskService;
using namespace Nexus::WebPortal;

RiskWebServlet::PortfolioSubscriber::PortfolioSubscriber(DirectoryEntry account,
  std::unique_ptr<WebSocketChannel> channel)
  : m_account(std::move(account)),
    m_client(std::move(channel)) {}

bool RiskWebServlet::PortfolioFilter::IsFiltered(
    const PortfolioModel::Entry& entry, const DirectoryEntry& group) const {
  if(m_groups.find(group) == m_groups.end()) {
    return true;
  }
  if(m_currencies.find(entry.m_inventory.m_position.m_key.m_currency) ==
      m_currencies.end()) {
    return true;
  }
  if(m_markets.find(entry.m_inventory.m_position.m_key.m_index.GetMarket()) ==
      m_markets.end()) {
    return true;
  }
  return false;
}

RiskWebServlet::RiskWebServlet(Ref<SessionStore<WebPortalSession>> sessions,
    ServiceClientsBox serviceClients)
    : m_sessions(sessions.Get()),
      m_serviceClients(std::move(serviceClients)),
      m_portfolioModel(m_serviceClients),
      m_portfolioTimer(m_serviceClients.BuildTimer(seconds(1))) {
  try {
    m_portfolioTimer.GetPublisher().Monitor(m_tasks.GetSlot<Timer::Result>(
      std::bind(&RiskWebServlet::OnPortfolioTimerExpired, this,
        std::placeholders::_1)));
    m_portfolioModel.GetPublisher().Monitor(
      m_tasks.GetSlot<PortfolioModel::Entry>(
        std::bind(&RiskWebServlet::OnPortfolioUpdate, this,
          std::placeholders::_1)));
    m_portfolioTimer.Start();
  } catch(const std::exception&) {
    Close();
    BOOST_RETHROW;
  }
}

RiskWebServlet::~RiskWebServlet() {
  Close();
}

std::vector<HttpRequestSlot> RiskWebServlet::GetSlots() {
  auto slots = std::vector<HttpRequestSlot>();
  return slots;
}

std::vector<HttpUpgradeSlot<RiskWebServlet::WebSocketChannel>>
    RiskWebServlet::GetWebSocketSlots() {
  auto slots = std::vector<HttpUpgradeSlot<WebSocketChannel>>();
  slots.emplace_back(MatchesPath(HttpMethod::GET,
    "/api/risk_service/portfolio"), std::bind(
      &RiskWebServlet::OnPortfolioUpgrade, this, std::placeholders::_1,
      std::placeholders::_2));
  return slots;
}

void RiskWebServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  m_portfolioTimer.Cancel();
  m_openState.Close();
}

const DirectoryEntry& RiskWebServlet::FindTradingGroup(
    const DirectoryEntry& trader) {
  auto groupIterator = m_traderGroups.find(trader);
  if(groupIterator != m_traderGroups.end()) {
    return groupIterator->second;
  }
  auto groups =
    m_serviceClients.GetAdministrationClient().LoadManagedTradingGroups(
      m_serviceClients.GetServiceLocatorClient().GetAccount());
  for(auto& group : groups) {
    auto tradingGroup =
      m_serviceClients.GetAdministrationClient().LoadTradingGroup(group);
    if(std::find(tradingGroup.GetManagers().begin(),
        tradingGroup.GetManagers().end(), trader) !=
        tradingGroup.GetManagers().end() ||
        std::find(tradingGroup.GetTraders().begin(),
          tradingGroup.GetTraders().end(), trader) !=
          tradingGroup.GetTraders().end()) {
      m_traderGroups.insert(std::make_pair(trader, tradingGroup.GetEntry()));
      return FindTradingGroup(trader);
    }
  }
  m_traderGroups.insert(std::make_pair(trader, DirectoryEntry{}));
  return FindTradingGroup(trader);
}

void RiskWebServlet::SendPortfolioEntry(const PortfolioModel::Entry& entry,
    const DirectoryEntry& group, PortfolioSubscriber& subscriber,
    bool checkFilter) {
  if(checkFilter) {
    if(subscriber.m_filter.IsFiltered(entry, group)) {
      return;
    }
  }
  auto sender = JsonSender<SharedBuffer>();
  auto entryFrame = StompFrame(StompCommand::MESSAGE);
  entryFrame.AddHeader({"subscription", subscriber.m_subscriptionId});
  entryFrame.AddHeader({"destination", "/api/risk_service/portfolio"});
  entryFrame.AddHeader({"content-type", "application/json"});
  auto buffer = Encode<SharedBuffer>(sender, entry);
  entryFrame.SetBody(std::move(buffer));
  subscriber.m_client.Write(entryFrame);
}

void RiskWebServlet::OnPortfolioUpgrade(const HttpRequest& request,
    std::unique_ptr<WebSocketChannel> channel) {
  auto session = m_sessions->Find(request);
  if(session == nullptr) {
    channel->GetConnection().Close();
    return;
  }
  auto subscriber = std::make_shared<PortfolioSubscriber>(
    session->GetAccount(), std::move(channel));
  Routines::Spawn(
    [=] {
      while(true) {
        auto frame = subscriber->m_client.Read();
        auto buffer = SharedBuffer();
        Serialize(frame, Store(buffer));
        std::cout << buffer << std::endl << std::endl << std::endl;
        if(frame.GetCommand() == StompCommand::SEND) {
          auto destination = *frame.FindHeader("destination");
          if(destination == "/api/risk_service/portfolio/filter") {
            OnPortfolioFilterRequest(subscriber, frame);
          } else {
            subscriber->m_client.Write(
              StompFrame::MakeDestinationNotFoundFrame(frame));
          }
        } else if(frame.GetCommand() == StompCommand::SUBSCRIBE) {
          auto destination = *frame.FindHeader("destination");
          if(destination == "/api/risk_service/portfolio") {
            OnPortfolioRequest(subscriber, frame);
          } else {
            subscriber->m_client.Write(
              StompFrame::MakeDestinationNotFoundFrame(frame));
          }
        }
      }
    });
}

void RiskWebServlet::OnPortfolioRequest(
    const std::shared_ptr<PortfolioSubscriber>& subscriber,
    const StompFrame& frame) {
  auto idHeader = *frame.FindHeader("id");
  if(!subscriber->m_subscriptionId.empty()) {
    subscriber->m_client.Write(StompFrame::MakeBadRequestFrame(frame,
      "Client already subscribed to portfolio."));
    return;
  }
  subscriber->m_subscriptionId = idHeader;
  m_tasks.Push(
    [=] {
      auto groups =
        m_serviceClients.GetAdministrationClient().LoadManagedTradingGroups(
        subscriber->m_account);
      std::move(groups.begin(), groups.end(), std::inserter(
        subscriber->m_filter.m_groups, subscriber->m_filter.m_groups.end()));
      auto currencyDatabase =
        m_serviceClients.GetDefinitionsClient().LoadCurrencyDatabase();
      for(auto& currency : currencyDatabase.GetEntries()) {
        subscriber->m_filter.m_currencies.insert(currency.m_id);
      }
      auto marketDatabase =
        m_serviceClients.GetDefinitionsClient().LoadMarketDatabase();
      for(auto& market : marketDatabase.GetEntries()) {
        subscriber->m_filter.m_markets.insert(market.m_code);
      }
      for(auto& entry : m_portfolioEntries) {
        auto& group = FindTradingGroup(entry.second.m_account);
        try {
          SendPortfolioEntry(entry.second, group, *subscriber, true);
        } catch(const std::exception&) {
          return;
        }
      }
      m_porfolioSubscribers.push_back(subscriber);
    });
}

void RiskWebServlet::OnPortfolioFilterRequest(
    const std::shared_ptr<PortfolioSubscriber>& subscriber,
    const StompFrame& frame) {
  struct Parameters {
    std::string m_id;
    std::vector<DirectoryEntry> m_groups;
    std::vector<CurrencyId> m_currencies;
    std::vector<MarketCode> m_markets;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
      shuttle.Shuttle("groups", m_groups);
      shuttle.Shuttle("currencies", m_currencies);
      shuttle.Shuttle("markets", m_markets);
    }
  };
  if(subscriber->m_subscriptionId.empty()) {
    return;
  }
  auto receiver = JsonReceiver<SharedBuffer>();
  auto parameters = Parameters();
  try {
    receiver.SetSource(Ref(frame.GetBody()));
    receiver.Shuttle(parameters);
  } catch(const std::exception&) {
    subscriber->m_client.Write(StompFrame::MakeBadRequestFrame(frame));
    return;
  }
  if(parameters.m_id != subscriber->m_subscriptionId) {
    subscriber->m_client.Write(StompFrame::MakeBadRequestFrame(frame,
      "Subscription id not found."));
    return;
  }
  auto managedGroupsList =
    m_serviceClients.GetAdministrationClient().LoadManagedTradingGroups(
    subscriber->m_account);
  std::sort(parameters.m_groups.begin(), parameters.m_groups.end());
  std::unordered_set<DirectoryEntry> managedGroups;
  std::move(managedGroupsList.begin(), managedGroupsList.end(),
    std::inserter(managedGroups, managedGroups.end()));
  auto updatedFilter = PortfolioFilter();
  set_intersection(managedGroups.begin(), managedGroups.end(),
    parameters.m_groups.begin(), parameters.m_groups.end(),
    std::inserter(updatedFilter.m_groups, updatedFilter.m_groups.end()));
  std::move(parameters.m_markets.begin(), parameters.m_markets.end(),
    std::inserter(updatedFilter.m_markets, updatedFilter.m_markets.end()));
  std::move(parameters.m_currencies.begin(), parameters.m_currencies.end(),
    std::inserter(updatedFilter.m_currencies,
    updatedFilter.m_currencies.end()));
  for(auto& entry : m_portfolioEntries) {
    auto& group = FindTradingGroup(entry.second.m_account);
    if(!subscriber->m_filter.IsFiltered(entry.second, group) &&
        updatedFilter.IsFiltered(entry.second, group)) {
      auto emptyEntry = entry.second;
      emptyEntry.m_unrealizedProfitAndLoss = Money::ZERO;
      emptyEntry.m_inventory.m_volume = 0;
      emptyEntry.m_inventory.m_transactionCount = 0;
      try {
        SendPortfolioEntry(emptyEntry, group, *subscriber, false);
      } catch(const std::exception&) {
        return;
      }
    } else if(subscriber->m_filter.IsFiltered(entry.second, group) &&
        !updatedFilter.IsFiltered(entry.second, group)) {
      try {
        SendPortfolioEntry(entry.second, group, *subscriber, false);
      } catch(const std::exception&) {
        return;
      }
    }
  }
  subscriber->m_filter = std::move(updatedFilter);
}

void RiskWebServlet::OnPortfolioUpdate(
    const PortfolioModel::Entry& entry) {
  m_updatedPortfolioEntries.insert(entry);
}

void RiskWebServlet::OnPortfolioTimerExpired(Timer::Result result) {
  if(result != Timer::Result::EXPIRED) {
    return;
  }
  auto updatedEntries = std::vector<PortfolioModel::Entry>();
  updatedEntries.reserve(m_updatedPortfolioEntries.size());
  std::move(m_updatedPortfolioEntries.begin(), m_updatedPortfolioEntries.end(),
    std::back_inserter(updatedEntries));
  for(auto& updatedEntry : updatedEntries) {
    auto key = RiskPortfolioKey(updatedEntry.m_account,
      updatedEntry.m_inventory.m_position.m_key.m_index);
    auto entryResult = m_portfolioEntries.insert(
      std::make_pair(key, updatedEntry));
    if(!entryResult.second) {
      entryResult.first->second = updatedEntry;
    }
  }
  m_updatedPortfolioEntries.clear();
  auto sender = JsonSender<SharedBuffer>();
  auto i = m_porfolioSubscribers.begin();
  while(i != m_porfolioSubscribers.end()) {
    auto& subscriber = *i;
    try {
      for(auto& entry : updatedEntries) {
        auto& group = FindTradingGroup(entry.m_account);
        SendPortfolioEntry(entry, group, *subscriber, true);
      }
      ++i;
    } catch(const std::exception&) {
      i = m_porfolioSubscribers.erase(i);
    }
  }
  m_portfolioTimer.Start();
}
