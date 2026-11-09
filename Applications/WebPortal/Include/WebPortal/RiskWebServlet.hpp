#ifndef NEXUS_RISK_WEB_SERVLET_HPP
#define NEXUS_RISK_WEB_SERVLET_HPP
#include <condition_variable>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Services/ServiceProtocolServer.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/HttpUpgradeSlot.hpp>
#include <Beam/WebServices/WebSessionStore.hpp>
#include "Nexus/RiskService/RiskServices.hpp"
#include "WebPortal/RiskPortfolioSubscription.hpp"
#include "WebPortal/WebPortalSession.hpp"
#include "WebPortal/WebSocketServerConnection.hpp"

namespace Nexus {

  /** Provides a web interface to the RiskService. */
  class RiskWebServlet {
    public:

      /** The type of WebSocketChannel used. */
      using WebSocketChannel = WebSocketServerConnection::Channel;

      /** Constructs a servlet using the authenticated web sessions. */
      explicit RiskWebServlet(
        Beam::Ref<Beam::WebSessionStore<WebPortalSession>> sessions);
      ~RiskWebServlet();

      std::vector<Beam::HttpRequestSlot> get_slots();
      std::vector<Beam::HttpUpgradeSlot<WebSocketChannel>>
        get_web_socket_slots();
      void close();

    private:
      struct RiskWebSession {
        std::shared_ptr<WebPortalSession> m_session;
        std::unique_ptr<RiskPortfolioSubscription> m_subscription;
        Beam::RoutineHandler m_updates;
      };
      using WebServiceProtocolServer = Beam::ServiceProtocolServer<
        WebSocketServerConnection*, Beam::JsonSender<Beam::SharedBuffer>,
        Beam::NullEncoder, std::unique_ptr<Beam::LiveTimer>, RiskWebSession>;

      Beam::WebSessionStore<WebPortalSession>* m_sessions;
      WebSocketServerConnection m_websocket_server;
      mutable std::mutex m_mutex;
      std::condition_variable m_session_condition;
      std::shared_ptr<WebPortalSession> m_pending_session;
      std::unique_ptr<WebServiceProtocolServer> m_protocol_server;
      Beam::OpenState m_open_state;

      RiskWebServlet(const RiskWebServlet&) = delete;
      RiskWebServlet& operator =(const RiskWebServlet&) = delete;
      void on_websocket_upgrade(const Beam::HttpRequest& request,
        std::unique_ptr<WebSocketChannel> channel);
      void on_client_accepted(
        WebServiceProtocolServer::ServiceProtocolClient& client);
      void on_client_closed(
        WebServiceProtocolServer::ServiceProtocolClient& client);
      void on_subscribe(Beam::RequestToken<
        WebServiceProtocolServer::ServiceProtocolClient,
        SubscribeRiskPortfolioUpdatesService>& request);
  };
}

#endif
