#ifndef NEXUS_WEB_PORTAL_SERVLET_HPP
#define NEXUS_WEB_PORTAL_SERVLET_HPP
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/WebServices/FileStore.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/HttpUpgradeSlot.hpp>
#include <Beam/WebServices/WebSessionStore.hpp>
#include <Beam/WebServices/WebSocketChannel.hpp>
#include "WebPortal/AdministrationWebServlet.hpp"
#include "WebPortal/ComplianceWebServlet.hpp"
#include "WebPortal/DefinitionsWebServlet.hpp"
#include "WebPortal/MarketDataWebServlet.hpp"
#include "WebPortal/RiskWebServlet.hpp"
#include "WebPortal/ServiceLocatorWebServlet.hpp"
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus {

  /** Implements a web servlet for Spire client services. */
  class WebPortalServlet {
    public:

      /** The type of WebSocketChannel used. */
      using WebSocketChannel = Beam::WebSocketChannel<
        std::shared_ptr<Beam::TcpSocketChannel>>;

      /**
       * Constructs a WebPortalServlet.
       * @param clients_builder The function used to build session Clients.
       * @param clients The clients used to access Spire services.
       */
      WebPortalServlet(ServiceLocatorWebServlet::ClientsBuilder clients_builder,
        Clients clients);

      ~WebPortalServlet();

      /** Returns the HTTP request slots. */
      std::vector<Beam::HttpRequestSlot> get_slots();

      /** Returns the WebSocket upgrade slots. */
      std::vector<Beam::HttpUpgradeSlot<WebSocketChannel>>
        get_web_socket_slots();

      void close();

    private:
      Beam::FileStore m_file_store;
      Beam::WebSessionStore<WebPortalSession> m_sessions;
      ServiceLocatorWebServlet m_service_locator_servlet;
      DefinitionsWebServlet m_definitions_servlet;
      AdministrationWebServlet m_administration_servlet;
      MarketDataWebServlet m_market_data_servlet;
      ComplianceWebServlet m_compliance_servlet;
      RiskWebServlet m_risk_servlet;
      Beam::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      WebPortalServlet(const WebPortalServlet&) = delete;
      WebPortalServlet& operator=(const WebPortalServlet&) = delete;
      Beam::HttpResponse on_index(const Beam::HttpRequest& request);
      Beam::HttpResponse on_serve_file(const Beam::HttpRequest& request);
  };
}

#endif
