#ifndef NEXUS_DEFINITIONS_WEB_SERVLET_HPP
#define NEXUS_DEFINITIONS_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include <boost/noncopyable.hpp>
#include "WebPortal/WebPortal.hpp"
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus::WebPortal {

  /** Provides a web interface to the DefinitionsService. */
  class DefinitionsWebServlet : private boost::noncopyable {
    public:

      /**
       * Constructs a DefinitionsWebServlet.
       * @param sessions The available web sessions.
       */
      explicit DefinitionsWebServlet(Beam::Ref<
        Beam::WebServices::SessionStore<WebPortalSession>> sessions);

      ~DefinitionsWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      void Close();

    private:
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      Beam::IO::OpenState m_openState;

      Beam::WebServices::HttpResponse OnLoadOrganizationName(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadComplianceRuleSchemas(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadCountryDatabase(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadCurrencyDatabase(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadDestinationDatabase(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadExchangeRates(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadMarketDatabase(
        const Beam::WebServices::HttpRequest& request);
  };
}

#endif
