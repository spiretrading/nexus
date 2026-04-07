#ifndef NEXUS_COMPLIANCE_WEB_SERVLET_HPP
#define NEXUS_COMPLIANCE_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/WebSessionStore.hpp>
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
        Beam::Ref<Beam::WebSessionStore<WebPortalSession>> sessions);

      ~ComplianceWebServlet();

      std::vector<Beam::HttpRequestSlot> get_slots();
      void close();

    private:
      Beam::WebSessionStore<WebPortalSession>* m_sessions;
      Beam::OpenState m_open_state;

      ComplianceWebServlet(const ComplianceWebServlet&) = delete;
      ComplianceWebServlet& operator=(const ComplianceWebServlet&) = delete;
      Beam::HttpResponse on_load_directory_entry_compliance_rule_entry(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_add_compliance_rule_entry(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_update_compliance_rule_entry(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_delete_compliance_rule_entry(
        const Beam::HttpRequest& request);
  };
}

#endif
