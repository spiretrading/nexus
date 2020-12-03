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
#include <Beam/WebServices/SessionStore.hpp>
#include <Beam/WebServices/WebSocketChannel.hpp>
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "WebPortal/AdministrationWebServlet.hpp"
#include "WebPortal/ComplianceWebServlet.hpp"
#include "WebPortal/DefinitionsWebServlet.hpp"
#include "WebPortal/MarketDataWebServlet.hpp"
#include "WebPortal/RiskWebServlet.hpp"
#include "WebPortal/ServiceLocatorWebServlet.hpp"
#include "WebPortal/WebPortal.hpp"
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus::WebPortal {

  /** Implements a web servlet for Spire client services. */
  class WebPortalServlet {
    public:

      /** The type of WebSocketChannel used. */
      using WebSocketChannel = Beam::WebServices::WebSocketChannel<
        std::shared_ptr<Beam::Network::TcpSocketChannel>>;

      /**
       * Constructs a WebPortalServlet.
       * @param serviceClientsBuilder The function used to build session
       *        ServiceClients.
       * @param serviceClients The clients used to access Spire services.
       */
      WebPortalServlet(
        ServiceLocatorWebServlet::ServiceClientsBuilder serviceClientsBuilder,
        ServiceClientsBox serviceClients);

      ~WebPortalServlet();

      /** Returns the HTTP request slots. */
      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      /** Returns the WebSocket upgrade slots. */
      std::vector<Beam::WebServices::HttpUpgradeSlot<WebSocketChannel>>
        GetWebSocketSlots();

      void Close();

    private:
      Beam::WebServices::FileStore m_fileStore;
      Beam::WebServices::SessionStore<WebPortalSession> m_sessions;
      ServiceLocatorWebServlet m_serviceLocatorServlet;
      DefinitionsWebServlet m_definitionsServlet;
      AdministrationWebServlet m_administrationServlet;
      MarketDataWebServlet m_marketDataServlet;
      ComplianceWebServlet m_complianceServlet;
      RiskWebServlet m_riskServlet;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      WebPortalServlet(const WebPortalServlet&) = delete;
      WebPortalServlet& operator =(const WebPortalServlet&) = delete;
      Beam::WebServices::HttpResponse OnIndex(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnServeFile(
        const Beam::WebServices::HttpRequest& request);
  };
}

#endif
