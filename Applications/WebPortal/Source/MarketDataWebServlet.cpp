#include "WebPortal/MarketDataWebServlet.hpp"
#include <limits>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include "WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

MarketDataWebServlet::MarketDataWebServlet(
    Ref<WebSessionStore<WebPortalSession>> sessions)
    : m_sessions(sessions.get()) {
  m_protocol_server =
    std::make_unique<WebServiceProtocolServer>(&m_websocket_server,
      [] { return std::make_unique<LiveTimer>(seconds(30)); },
      std::bind_front(&MarketDataWebServlet::on_client_accepted, this),
      std::bind_front(&MarketDataWebServlet::on_client_closed, this));
  Nexus::register_query_types(
    out(m_protocol_server->get_slots().get_registry()));
  register_market_data_registry_services(out(m_protocol_server->get_slots()));
  register_market_data_registry_messages(out(m_protocol_server->get_slots()));
  QueryBboQuotesService::add_request_slot(out(m_protocol_server->get_slots()),
    std::bind_front(&MarketDataWebServlet::on_query_bbo_quotes, this));
  add_message_slot<EndBboQuoteQueryMessage>(out(m_protocol_server->get_slots()),
    std::bind_front(&MarketDataWebServlet::on_end_bbo_quote_query, this));
}

MarketDataWebServlet::~MarketDataWebServlet() {
  close();
}

std::vector<HttpRequestSlot> MarketDataWebServlet::get_slots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(matches_path(HttpMethod::POST,
    "/api/market_data_service/load_ticker_info_from_prefix"), std::bind_front(
      &MarketDataWebServlet::on_load_ticker_info_from_prefix, this));
  return slots;
}

std::vector<HttpUpgradeSlot<MarketDataWebServlet::WebSocketChannel>>
    MarketDataWebServlet::get_web_socket_slots() {
  auto slots = std::vector<HttpUpgradeSlot<WebSocketChannel>>();
  slots.emplace_back(
    matches_path(HttpMethod::GET, "/api/market_data_service/websocket"),
    std::bind_front(&MarketDataWebServlet::on_websocket_upgrade, this));
  return slots;
}

void MarketDataWebServlet::close() {
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

HttpResponse MarketDataWebServlet::on_load_ticker_info_from_prefix(
    const HttpRequest& request) {
  struct Parameters {
    std::string m_prefix;

    void shuttle(JsonReceiver<SharedBuffer>& shuttle, unsigned int version) {
      shuttle.shuttle("prefix", m_prefix);
    }
  };
  auto response = HttpResponse();
  auto session = m_sessions->find(request);
  if(!session) {
    response.set_status_code(HttpStatusCode::UNAUTHORIZED);
    return response;
  }
  auto parameters = session->shuttle_parameters<Parameters>(request);
  auto& clients = session->get_clients();
  auto ticker_infos =
    clients.get_market_data_client().load_ticker_info_from_prefix(
      parameters.m_prefix);
  session->shuttle_response(ticker_infos, out(response));
  return response;
}

void MarketDataWebServlet::on_websocket_upgrade(
    const HttpRequest& request, std::unique_ptr<WebSocketChannel> channel) {
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

void MarketDataWebServlet::on_client_accepted(
    WebServiceProtocolServer::ServiceProtocolClient& client) {
  auto lock = std::lock_guard(m_mutex);
  client.get_session().m_session = std::move(m_pending_session);
  m_session_condition.notify_all();
  if(!client.get_session().m_session) {
    throw EndOfFileException();
  }
}

void MarketDataWebServlet::on_client_closed(
    WebServiceProtocolServer::ServiceProtocolClient& client) {
  auto& subscriptions = client.get_session().m_subscriptions;
  for(auto& [id, entry] : subscriptions) {
    entry.m_subscription->close();
  }
  for(auto& [id, entry] : subscriptions) {
    entry.m_updates.wait();
  }
  subscriptions.clear();
}

void MarketDataWebServlet::on_query_bbo_quotes(
    RequestToken<WebServiceProtocolServer::ServiceProtocolClient,
      QueryBboQuotesService>& request, const TickerQuery& query) {
  auto& session = request.get_session();
  if(!session.m_session->is_logged_in() || session.m_session->is_expired()) {
    throw ServiceRequestException("Session expired.");
  }
  if(session.m_next_id == std::numeric_limits<int>::max()) {
    throw ServiceRequestException("Query IDs exhausted.");
  }
  auto subscription = std::make_unique<BboQuoteSubscription>(
    session.m_session->get_clients().get_market_data_client(),
    query);
  auto snapshot = subscription->get_snapshot();
  if(query.get_range().get_end() != Beam::Sequence::LAST) {
    request.set(BboQuoteQueryResult(-1, std::move(snapshot)));
    return;
  }
  auto id = session.m_next_id++;
  auto& entry = session.m_subscriptions[id];
  entry.m_subscription = std::move(subscription);
  request.set(BboQuoteQueryResult(id, std::move(snapshot)));
  auto client = &request.get_client();
  auto stream = entry.m_subscription.get();
  entry.m_updates = spawn([=] {
    try {
      while(true) {
        auto value = stream->pop();
        auto& session = *client->get_session().m_session;
        if(!session.is_logged_in() || session.is_expired()) {
          throw EndOfFileException();
        }
        send_record_message<BboQuoteMessage>(*client,
          SequencedTickerBboQuote(TickerBboQuote(*value, stream->get_ticker()),
            value.get_sequence()));
      }
    } catch(const std::exception&) {
      if(!stream->is_closed()) {
        client->close();
      }
    }
  });
}

void MarketDataWebServlet::on_end_bbo_quote_query(
    WebServiceProtocolServer::ServiceProtocolClient& client,
    const Ticker& ticker, int id) {
  auto& subscriptions = client.get_session().m_subscriptions;
  auto i = subscriptions.find(id);
  if(i == subscriptions.end() ||
      i->second.m_subscription->get_ticker() != ticker) {
    return;
  }
  i->second.m_subscription->close();
  i->second.m_updates.wait();
  subscriptions.erase(i);
}
