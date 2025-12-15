#include "WebPortal/RiskWebServlet.hpp"
#include <algorithm>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

RiskWebServlet::PortfolioSubscriber::PortfolioSubscriber(
  DirectoryEntry account, std::unique_ptr<WebSocketChannel> channel)
  : m_account(std::move(account)) {}

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
    Ref<WebSessionStore<WebPortalSession>> sessions, Clients clients)
  : m_clients(std::move(clients)),
    m_sessions(sessions.get()),
    m_portfolio_model(m_clients),
    m_portfolio_timer(m_clients.make_timer(seconds(1))) {
  try {
    m_portfolio_timer.get_publisher().monitor(m_tasks.get_slot<Timer::Result>(
      std::bind_front(&RiskWebServlet::on_portfolio_timer_expired, this)));
    m_portfolio_model.get_publisher().monitor(
      m_tasks.get_slot<PortfolioModel::Entry>(
        std::bind_front(&RiskWebServlet::on_portfolio_update, this)));
    m_portfolio_timer.start();
  } catch(const std::exception&) {
    close();
    throw;
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
    matches_path(HttpMethod::GET, "/api/risk_service/portfolio"),
    std::bind_front(&RiskWebServlet::on_portfolio_upgrade, this));
  return slots;
}

void RiskWebServlet::close() {
  if(m_open_state.set_closing()) {
    return;
  }
  m_portfolio_timer.cancel();
  m_open_state.close();
}

const DirectoryEntry& RiskWebServlet::find_trading_group(
    const DirectoryEntry& trader) {
  auto group_iterator = m_trader_groups.find(trader);
  if(group_iterator != m_trader_groups.end()) {
    return group_iterator->second;
  }
  auto groups =
    m_clients.get_administration_client().load_managed_trading_groups(
      m_clients.get_service_locator_client().get_account());
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
}

void RiskWebServlet::on_portfolio_upgrade(const HttpRequest& request,
    std::unique_ptr<WebSocketChannel> channel) {
  auto session = m_sessions->find(request);
  if(!session) {
    channel->get_connection().close();
    return;
  }
  auto subscriber = std::make_shared<PortfolioSubscriber>(
    session->get_account(), std::move(channel));
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
  std::move(m_updated_portfolio_entries.begin(),
    m_updated_portfolio_entries.end(), std::back_inserter(updated_entries));
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
  m_portfolio_timer.start();
}
