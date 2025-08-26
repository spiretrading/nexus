#ifndef NEXUS_DEFINITIONS_WEB_SERVLET_HPP
#define NEXUS_DEFINITIONS_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus {

  /** Provides a web interface to the DefinitionsService. */
  class DefinitionsWebServlet {
    public:

      /**
       * Constructs a DefinitionsWebServlet.
       * @param sessions The available web sessions.
       */
      explicit DefinitionsWebServlet(
        Beam::Ref<Beam::WebServices::SessionStore<WebPortalSession>> sessions);

      ~DefinitionsWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> get_slots();

      void close();

    private:
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      Beam::IO::OpenState m_open_state;

      DefinitionsWebServlet(const DefinitionsWebServlet&) = delete;
      DefinitionsWebServlet& operator=(const DefinitionsWebServlet&) = delete;
      Beam::WebServices::HttpResponse on_load_organization_name(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_compliance_rule_schemas(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_country_database(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_currency_database(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_destination_database(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_exchange_rates(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_venue_database(
        const Beam::WebServices::HttpRequest& request);
  };
}

#endif
