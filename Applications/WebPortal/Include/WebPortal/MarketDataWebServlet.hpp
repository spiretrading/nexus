#ifndef NEXUS_MARKET_DATA_WEB_SERVLET_HPP
#define NEXUS_MARKET_DATA_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
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
        Beam::Ref<Beam::WebServices::SessionStore<WebPortalSession>> sessions);

      ~MarketDataWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> get_slots();

      void close();

    private:
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      Beam::IO::OpenState m_open_state;

      MarketDataWebServlet(const MarketDataWebServlet&) = delete;
      MarketDataWebServlet& operator=(const MarketDataWebServlet&) = delete;
      Beam::WebServices::HttpResponse on_load_security_info_from_prefix(
        const Beam::WebServices::HttpRequest& request);
  };
}

#endif
