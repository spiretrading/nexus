#ifndef NEXUS_CLIENTWEBPORTALSERVLET_HPP
#define NEXUS_CLIENTWEBPORTALSERVLET_HPP
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
#include <boost/noncopyable.hpp>
#include "ClientWebPortal/ClientWebPortal/AdministrationWebServlet.hpp"
#include "ClientWebPortal/ClientWebPortal/ClientWebPortal.hpp"
#include "ClientWebPortal/ClientWebPortal/ClientWebPortalSession.hpp"
#include "ClientWebPortal/ClientWebPortal/ComplianceWebServlet.hpp"
#include "ClientWebPortal/ClientWebPortal/DefinitionsWebServlet.hpp"
#include "ClientWebPortal/ClientWebPortal/MarketDataWebServlet.hpp"
#include "ClientWebPortal/ClientWebPortal/RiskWebServlet.hpp"
#include "ClientWebPortal/ClientWebPortal/ServiceLocatorWebServlet.hpp"
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"

namespace Nexus {
namespace ClientWebPortal {

  /*! \class ClientWebPortalServlet
      \brief Implements a web servlet for Spire client services.
   */
  class ClientWebPortalServlet : private boost::noncopyable {
    public:

      //! The type of WebSocketChannel used.
      using WebSocketChannel = Beam::WebServices::WebSocketChannel<
        std::shared_ptr<Beam::Network::TcpSocketChannel>>;

      //! Constructs a ClientWebPortalServlet.
      /*!
        \param serviceClients The clients used to access Spire services.
      */
      ClientWebPortalServlet(
        Beam::RefType<ApplicationServiceClients> serviceClients);

      ~ClientWebPortalServlet();

      //! Returns the HTTP request slots.
      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      //! Returns the WebSocket upgrade slots.
      std::vector<Beam::WebServices::HttpUpgradeSlot<WebSocketChannel>>
        GetWebSocketSlots();

      void Open();

      void Close();

    private:
      Beam::WebServices::FileStore m_fileStore;
      Beam::WebServices::SessionStore<ClientWebPortalSession> m_sessions;
      ApplicationServiceClients* m_serviceClients;
      ServiceLocatorWebServlet m_serviceLocatorServlet;
      DefinitionsWebServlet m_definitionsServlet;
      AdministrationWebServlet m_administrationServlet;
      MarketDataWebServlet m_marketDataServlet;
      ComplianceWebServlet m_complianceServlet;
      RiskWebServlet m_riskServlet;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      Beam::WebServices::HttpResponse OnIndex(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnServeFile(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadProfitAndLossReport(
        const Beam::WebServices::HttpRequest& request);
  };
}
}

#endif
