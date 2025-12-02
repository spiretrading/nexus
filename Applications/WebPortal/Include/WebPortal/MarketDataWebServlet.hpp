#ifndef NEXUS_MARKET_DATA_WEB_SERVLET_HPP
#define NEXUS_MARKET_DATA_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/WebSessionStore.hpp>
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus {

  /** Provides a web interface to the MarketDataService. */
  class MarketDataWebServlet {
    public:

      /**
       * Constructs a MarketDataWebServlet.
       * @param sessions The available web sessions.
       */
      explicit MarketDataWebServlet(
        Beam::Ref<Beam::WebSessionStore<WebPortalSession>> sessions);

      ~MarketDataWebServlet();

      std::vector<Beam::HttpRequestSlot> get_slots();
      void close();

    private:
      Beam::WebSessionStore<WebPortalSession>* m_sessions;
      Beam::OpenState m_open_state;

      MarketDataWebServlet(const MarketDataWebServlet&) = delete;
      MarketDataWebServlet& operator=(const MarketDataWebServlet&) = delete;
      Beam::HttpResponse on_load_security_info_from_prefix(
        const Beam::HttpRequest& request);
  };
}

#endif
