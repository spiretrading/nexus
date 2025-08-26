#ifndef NEXUS_COMPLIANCE_WEB_SERVLET_HPP
#define NEXUS_COMPLIANCE_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus {

  /** Provides a web interface to the ComplianceService. */
  class ComplianceWebServlet {
    public:

      /**
       * Constructs a ComplianceWebServlet.
       * @param sessions The available web sessions.
       */
      explicit ComplianceWebServlet(
        Beam::Ref<Beam::WebServices::SessionStore<WebPortalSession>> sessions);

      ~ComplianceWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> get_slots();

      void close();

    private:
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      Beam::IO::OpenState m_open_state;

      ComplianceWebServlet(const ComplianceWebServlet&) = delete;
      ComplianceWebServlet& operator=(const ComplianceWebServlet&) = delete;
      Beam::WebServices::HttpResponse
        on_load_directory_entry_compliance_rule_entry(
          const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_add_compliance_rule_entry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_update_compliance_rule_entry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_delete_compliance_rule_entry(
        const Beam::WebServices::HttpRequest& request);
  };
}

#endif
