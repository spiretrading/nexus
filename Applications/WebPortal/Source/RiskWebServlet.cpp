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

RiskWebServlet::PortfolioSubscriber::PortfolioSubscriber(
  DirectoryEntry account, std::unique_ptr<WebSocketChannel> channel)
  : m_account(std::move(account)),
    m_client(std::move(channel)) {}

bool RiskWebServlet::PortfolioFilter::is_filtered(
    const PortfolioModel::Entry& entry, const DirectoryEntry& group) const {
  if(m_groups.find(group) == m_groups.end()) {
    return true;
  }
  if(m_currencies.find(entry.m_inventory.m_position.m_currency) ==
      m_currencies.end()) {
    return true;
  }
  if(m_venues.find(entry.m_inventory.m_position.m_security.get_venue()) ==
      m_venues.end()) {
    return true;
  }
  return false;
}

RiskWebServlet::RiskWebServlet(
    Ref<SessionStore<WebPortalSession>> sessions, Clients clients)
  : m_clients(std::move(clients)),
    m_sessions(sessions.Get()),
    m_portfolio_model(m_clients),
    m_portfolio_timer(m_clients.make_timer(seconds(1))) {
  try {
    m_portfolio_timer.GetPublisher().Monitor(m_tasks.GetSlot<Timer::Result>(
      std::bind_front(&RiskWebServlet::on_portfolio_timer_expired, this)));
    m_portfolio_model.get_publisher().Monitor(
      m_tasks.GetSlot<PortfolioModel::Entry>(
        std::bind_front(&RiskWebServlet::on_portfolio_update, this)));
    m_portfolio_timer.Start();
  } catch(const std::exception&) {
    close();
    BOOST_RETHROW;
  }
}

RiskWebServlet::~RiskWebServlet() {
  close();
}

std::vector<HttpRequestSlot> RiskWebServlet::get_slots() {
  return {};
}

std::vector<HttpUpgradeSlot<RiskWebServlet::WebSocketChannel>>
    RiskWebServlet::get_web_socket_slots() {
  auto slots = std::vector<HttpUpgradeSlot<WebSocketChannel>>();
  slots.emplace_back(
    MatchesPath(HttpMethod::GET, "/api/risk_service/portfolio"),
    std::bind_front(&RiskWebServlet::on_portfolio_upgrade, this));
  return slots;
}

void RiskWebServlet::close() {
  if(m_open_state.SetClosing()) {
    return;
  }
  m_portfolio_timer.Cancel();
  m_open_state.Close();
}

const DirectoryEntry& RiskWebServlet::find_trading_group(
    const DirectoryEntry& trader) {
  auto group_iterator = m_trader_groups.find(trader);
  if(group_iterator != m_trader_groups.end()) {
    return group_iterator->second;
  }
  auto groups =
    m_clients.get_administration_client().load_managed_trading_groups(
      m_clients.get_service_locator_client().GetAccount());
  for(auto& group : groups) {
    auto trading_group =
      m_clients.get_administration_client().load_trading_group(group);
    if(std::find(trading_group.get_managers().begin(),
        trading_group.get_managers().end(), trader) !=
        trading_group.get_managers().end() ||
        std::find(trading_group.get_traders().begin(),
          trading_group.get_traders().end(), trader) !=
          trading_group.get_traders().end()) {
      m_trader_groups.insert(std::make_pair(trader, trading_group.get_entry()));
      return find_trading_group(trader);
    }
  }
  m_trader_groups.insert(std::make_pair(trader, DirectoryEntry{}));
  return find_trading_group(trader);
}

void RiskWebServlet::send_portfolio_entry(const PortfolioModel::Entry& entry,
    const DirectoryEntry& group, PortfolioSubscriber& subscriber,
    bool check_filter) {
  if(check_filter && subscriber.m_filter.is_filtered(entry, group)) {
    return;
  }
  auto sender = JsonSender<SharedBuffer>();
  auto entry_frame = StompFrame(StompCommand::MESSAGE);
  entry_frame.AddHeader({"subscription", subscriber.m_subscription_id});
  entry_frame.AddHeader({"destination", "/api/risk_service/portfolio"});
  entry_frame.AddHeader({"content-type", "application/json"});
  auto buffer = Encode<SharedBuffer>(sender, entry);
  entry_frame.SetBody(std::move(buffer));
  subscriber.m_client.Write(entry_frame);
}

void RiskWebServlet::on_portfolio_upgrade(const HttpRequest& request,
    std::unique_ptr<WebSocketChannel> channel) {
  auto session = m_sessions->Find(request);
  if(!session) {
    channel->GetConnection().Close();
    return;
  }
  auto subscriber = std::make_shared<PortfolioSubscriber>(
    session->GetAccount(), std::move(channel));
  Routines::Spawn([=, this] {
    while(true) {
      auto frame = subscriber->m_client.Read();
      auto buffer = SharedBuffer();
      Serialize(frame, Store(buffer));
      if(frame.GetCommand() == StompCommand::SEND) {
        auto destination = *frame.FindHeader("destination");
        if(destination == "/api/risk_service/portfolio/filter") {
          on_portfolio_filter_request(subscriber, frame);
        } else {
          subscriber->m_client.Write(
            StompFrame::MakeDestinationNotFoundFrame(frame));
        }
      } else if(frame.GetCommand() == StompCommand::SUBSCRIBE) {
        auto destination = *frame.FindHeader("destination");
        if(destination == "/api/risk_service/portfolio") {
          on_portfolio_request(subscriber, frame);
        } else {
          subscriber->m_client.Write(
            StompFrame::MakeDestinationNotFoundFrame(frame));
        }
      }
    }
  });
}

void RiskWebServlet::on_portfolio_request(
    const std::shared_ptr<PortfolioSubscriber>& subscriber,
    const StompFrame& frame) {
  auto id_header = *frame.FindHeader("id");
  if(!subscriber->m_subscription_id.empty()) {
    subscriber->m_client.Write(StompFrame::MakeBadRequestFrame(
      frame, "Client already subscribed to portfolio."));
    return;
  }
  subscriber->m_subscription_id = id_header;
  m_tasks.Push([=, this] {
    auto groups =
      m_clients.get_administration_client().load_managed_trading_groups(
        subscriber->m_account);
    std::move(groups.begin(), groups.end(), std::inserter(
      subscriber->m_filter.m_groups, subscriber->m_filter.m_groups.end()));
    auto currencies =
      m_clients.get_definitions_client().load_currency_database();
    for(auto& currency : currencies.get_entries()) {
      subscriber->m_filter.m_currencies.insert(currency.m_id);
    }
    auto venues = m_clients.get_definitions_client().load_venue_database();
    for(auto& venue : venues.get_entries()) {
      subscriber->m_filter.m_venues.insert(venue.m_venue);
    }
    for(auto& entry : m_portfolio_entries) {
      auto& group = find_trading_group(entry.second.m_account);
      try {
        send_portfolio_entry(entry.second, group, *subscriber, true);
      } catch(const std::exception&) {
        return;
      }
    }
    m_portfolio_subscribers.push_back(subscriber);
  });
}

void RiskWebServlet::on_portfolio_filter_request(
    const std::shared_ptr<PortfolioSubscriber>& subscriber,
    const StompFrame& frame) {
  struct Parameters {
    std::string m_id;
    std::vector<DirectoryEntry> m_groups;
    std::vector<CurrencyId> m_currencies;
    std::vector<Venue> m_venues;

    void Shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.Shuttle("id", m_id);
      shuttle.Shuttle("groups", m_groups);
      shuttle.Shuttle("currencies", m_currencies);
      shuttle.Shuttle("venues", m_venues);
    }
  };
  if(subscriber->m_subscription_id.empty()) {
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
  if(parameters.m_id != subscriber->m_subscription_id) {
    subscriber->m_client.Write(StompFrame::MakeBadRequestFrame(frame,
      "Subscription id not found."));
    return;
  }
  auto managed_groups_list =
    m_clients.get_administration_client().load_managed_trading_groups(
      subscriber->m_account);
  std::sort(parameters.m_groups.begin(), parameters.m_groups.end());
  auto managed_groups = std::unordered_set<DirectoryEntry>();
  std::move(managed_groups_list.begin(), managed_groups_list.end(),
    std::inserter(managed_groups, managed_groups.end()));
  auto updated_filter = PortfolioFilter();
  set_intersection(managed_groups.begin(), managed_groups.end(),
    parameters.m_groups.begin(), parameters.m_groups.end(),
    std::inserter(updated_filter.m_groups, updated_filter.m_groups.end()));
  std::move(parameters.m_venues.begin(), parameters.m_venues.end(),
    std::inserter(updated_filter.m_venues, updated_filter.m_venues.end()));
  std::move(parameters.m_currencies.begin(), parameters.m_currencies.end(),
    std::inserter(
      updated_filter.m_currencies, updated_filter.m_currencies.end()));
  for(auto& entry : m_portfolio_entries) {
    auto& group = find_trading_group(entry.second.m_account);
    if(!subscriber->m_filter.is_filtered(entry.second, group) &&
        updated_filter.is_filtered(entry.second, group)) {
      auto empty_entry = entry.second;
      empty_entry.m_unrealized_profit_and_loss = Money::ZERO;
      empty_entry.m_inventory.m_volume = 0;
      empty_entry.m_inventory.m_transaction_count = 0;
      try {
        send_portfolio_entry(empty_entry, group, *subscriber, false);
      } catch(const std::exception&) {
        return;
      }
    } else if(subscriber->m_filter.is_filtered(entry.second, group) &&
        !updated_filter.is_filtered(entry.second, group)) {
      try {
        send_portfolio_entry(entry.second, group, *subscriber, false);
      } catch(const std::exception&) {
        return;
      }
    }
  }
  subscriber->m_filter = std::move(updated_filter);
}

void RiskWebServlet::on_portfolio_update(const PortfolioModel::Entry& entry) {
  m_updated_portfolio_entries.insert(entry);
}

void RiskWebServlet::on_portfolio_timer_expired(Timer::Result result) {
  if(result != Timer::Result::EXPIRED) {
    return;
  }
  auto updated_entries = std::vector<PortfolioModel::Entry>();
  updated_entries.reserve(m_updated_portfolio_entries.size());
  std::move(m_updated_portfolio_entries.begin(), m_updated_portfolio_entries.end(),
    std::back_inserter(updated_entries));
  for(auto& updated_entry : updated_entries) {
    auto key = RiskPortfolioKey(
      updated_entry.m_account, updated_entry.m_inventory.m_position.m_security);
    auto entry_result =
      m_portfolio_entries.insert(std::pair(key, updated_entry));
    if(!entry_result.second) {
      entry_result.first->second = updated_entry;
    }
  }
  m_updated_portfolio_entries.clear();
  auto sender = JsonSender<SharedBuffer>();
  auto i = m_portfolio_subscribers.begin();
  while(i != m_portfolio_subscribers.end()) {
    auto& subscriber = *i;
    try {
      for(auto& entry : updated_entries) {
        auto& group = find_trading_group(entry.m_account);
        send_portfolio_entry(entry, group, *subscriber, true);
      }
      ++i;
    } catch(const std::exception&) {
      i = m_portfolio_subscribers.erase(i);
    }
  }
  m_portfolio_timer.Start();
}
