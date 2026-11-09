#ifndef NEXUS_MARKET_DATA_WEB_SERVLET_HPP
#define NEXUS_MARKET_DATA_WEB_SERVLET_HPP
#include <condition_variable>
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Services/ServiceProtocolServer.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/HttpUpgradeSlot.hpp>
#include <Beam/WebServices/WebSessionStore.hpp>
#include "Nexus/MarketDataService/MarketDataRegistryServices.hpp"
#include "WebPortal/BboQuoteSubscription.hpp"
#include "WebPortal/WebPortalSession.hpp"
#include "WebPortal/WebSocketServerConnection.hpp"

namespace Nexus {

  /** Provides a web interface to the MarketDataService. */
  class MarketDataWebServlet {
    public:

      /** The type of WebSocketChannel used. */
      using WebSocketChannel = WebSocketServerConnection::Channel;

      /**
       * Constructs a MarketDataWebServlet.
       * @param sessions The available web sessions.
       */
      explicit MarketDataWebServlet(
        Beam::Ref<Beam::WebSessionStore<WebPortalSession>> sessions);

      ~MarketDataWebServlet();

      std::vector<Beam::HttpRequestSlot> get_slots();
      std::vector<Beam::HttpUpgradeSlot<WebSocketChannel>>
        get_web_socket_slots();
      void close();

    private:
      struct Subscription {
        std::unique_ptr<BboQuoteSubscription> m_subscription;
        Beam::RoutineHandler m_updates;
      };
      struct Session {
        std::shared_ptr<WebPortalSession> m_session;
        std::unordered_map<int, Subscription> m_subscriptions;
        int m_next_id = 0;
      };
      using WebServiceProtocolServer = Beam::ServiceProtocolServer<
        WebSocketServerConnection*, Beam::JsonSender<Beam::SharedBuffer>,
        Beam::NullEncoder, std::unique_ptr<Beam::LiveTimer>, Session>;

      Beam::WebSessionStore<WebPortalSession>* m_sessions;
      WebSocketServerConnection m_websocket_server;
      mutable std::mutex m_mutex;
      std::condition_variable m_session_condition;
      std::shared_ptr<WebPortalSession> m_pending_session;
      std::unique_ptr<WebServiceProtocolServer> m_protocol_server;
      Beam::OpenState m_open_state;

      MarketDataWebServlet(const MarketDataWebServlet&) = delete;
      MarketDataWebServlet& operator=(const MarketDataWebServlet&) = delete;
      Beam::HttpResponse on_load_ticker_info_from_prefix(
        const Beam::HttpRequest& request);
      void on_websocket_upgrade(const Beam::HttpRequest& request,
        std::unique_ptr<WebSocketChannel> channel);
      void on_client_accepted(
        WebServiceProtocolServer::ServiceProtocolClient& client);
      void on_client_closed(
        WebServiceProtocolServer::ServiceProtocolClient& client);
      void on_query_bbo_quotes(Beam::RequestToken<
        WebServiceProtocolServer::ServiceProtocolClient,
        QueryBboQuotesService>& request, const TickerQuery& query);
      void on_end_bbo_quote_query(
        WebServiceProtocolServer::ServiceProtocolClient& client,
        const Ticker& ticker, int id);
  };
}

#endif
