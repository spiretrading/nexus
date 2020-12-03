#ifndef NEXUS_MARKET_DATA_WEB_SERVLET_HPP
#define NEXUS_MARKET_DATA_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include "WebPortal/WebPortal.hpp"
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus::WebPortal {

  /** Provides a web interface to the MarketDataService. */
  class MarketDataWebServlet {
    public:

      /**
       * Constructs a MarketDataWebServlet.
       * @param sessions The available web sessions.
       * @param serviceClients The clients used to access Spire services.
       */
      MarketDataWebServlet(Beam::Ref<
        Beam::WebServices::SessionStore<WebPortalSession>> sessions);

      ~MarketDataWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      void Close();

    private:
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      Beam::IO::OpenState m_openState;

      MarketDataWebServlet(const MarketDataWebServlet&) = delete;
      MarketDataWebServlet& operator =(const MarketDataWebServlet&) = delete;
      Beam::WebServices::HttpResponse OnLoadSecurityInfoFromPrefix(
        const Beam::WebServices::HttpRequest& request);
  };
}

#endif
