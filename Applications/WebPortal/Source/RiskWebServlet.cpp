#include "WebPortal/RiskWebServlet.hpp"
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

RiskWebServlet::RiskWebServlet(
    Ref<WebSessionStore<WebPortalSession>> sessions)
    : m_sessions(sessions.get()) {
  m_protocol_server =
    std::make_unique<WebServiceProtocolServer>(&m_websocket_server,
      [] { return std::make_unique<LiveTimer>(seconds(30)); },
      std::bind_front(&RiskWebServlet::on_client_accepted, this),
      std::bind_front(&RiskWebServlet::on_client_closed, this));
  Nexus::register_query_types(
    out(m_protocol_server->get_slots().get_registry()));
  register_risk_services(out(m_protocol_server->get_slots()));
  register_risk_messages(out(m_protocol_server->get_slots()));
  SubscribeRiskPortfolioUpdatesService::add_request_slot(
    out(m_protocol_server->get_slots()),
    std::bind_front(&RiskWebServlet::on_subscribe, this));
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
    matches_path(HttpMethod::GET, "/api/risk_service/websocket"),
    std::bind_front(&RiskWebServlet::on_websocket_upgrade, this));
  return slots;
}

void RiskWebServlet::close() {
  if(m_open_state.set_closing()) {
    return;
  }
  {
    auto lock = std::lock_guard(m_mutex);
    m_pending_session.reset();
    m_session_condition.notify_all();
  }
  m_websocket_server.close();
  m_protocol_server->close();
  m_open_state.close();
}

void RiskWebServlet::on_websocket_upgrade(const HttpRequest& request,
    std::unique_ptr<WebSocketChannel> channel) {
  auto session = m_sessions->find(request);
  if(!session || !session->is_logged_in() || session->is_expired()) {
    channel->get_connection().close();
    return;
  }
  channel->get_socket().set_binary_mode();
  auto lock = std::unique_lock(m_mutex);
  m_session_condition.wait(lock, [&] {
    return !m_pending_session || !m_open_state.is_open();
  });
  if(!m_open_state.is_open()) {
    channel->get_connection().close();
    return;
  }
  m_pending_session = session;
  m_websocket_server.push(std::move(channel));
  m_session_condition.wait(lock, [&] {
    return m_pending_session != session || !m_open_state.is_open();
  });
}

void RiskWebServlet::on_client_accepted(
    WebServiceProtocolServer::ServiceProtocolClient& client) {
  auto lock = std::lock_guard(m_mutex);
  client.get_session().m_session = std::move(m_pending_session);
  m_session_condition.notify_all();
  if(!client.get_session().m_session) {
    throw EndOfFileException();
  }
}

void RiskWebServlet::on_client_closed(
    WebServiceProtocolServer::ServiceProtocolClient& client) {
  auto& session = client.get_session();
  if(session.m_subscription) {
    session.m_subscription->close();
    session.m_updates.wait();
  }
}

void RiskWebServlet::on_subscribe(
    RequestToken<WebServiceProtocolServer::ServiceProtocolClient,
      SubscribeRiskPortfolioUpdatesService>& request) {
  auto& session = request.get_session();
  if(!session.m_session->is_logged_in() || session.m_session->is_expired()) {
    throw ServiceRequestException("Session expired.");
  }
  if(session.m_subscription) {
    throw ServiceRequestException("Already subscribed to portfolio updates.");
  }
  auto& publisher = session.m_session->get_clients().get_risk_client().
    get_risk_portfolio_update_publisher();
  session.m_subscription =
    std::make_unique<RiskPortfolioSubscription>(publisher);
  request.set(session.m_subscription->get_snapshot());
  auto client = &request.get_client();
  auto subscription = session.m_subscription.get();
  session.m_updates = spawn([=] {
    try {
      while(true) {
        auto entry = subscription->pop();
        auto& session = *client->get_session().m_session;
        if(!session.is_logged_in() || session.is_expired()) {
          throw EndOfFileException();
        }
        auto updates = std::vector<InventoryUpdate>();
        updates.emplace_back(entry.m_key.m_account, entry.m_value);
        send_record_message<InventoryMessage>(*client, updates);
      }
    } catch(const std::exception&) {
      client->close();
    }
  });
}
