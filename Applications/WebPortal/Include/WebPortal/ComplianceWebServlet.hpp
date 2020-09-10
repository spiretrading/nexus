#ifndef NEXUS_COMPLIANCE_WEB_SERVLET_HPP
#define NEXUS_COMPLIANCE_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include <boost/noncopyable.hpp>
#include "WebPortal/WebPortal.hpp"
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus::WebPortal {

  /** Provides a web interface to the ComplianceService. */
  class ComplianceWebServlet : private boost::noncopyable {
    public:

      /**
       * Constructs a ComplianceWebServlet.
       * @param sessions The available web sessions.
       */
      explicit ComplianceWebServlet(Beam::Ref<
        Beam::WebServices::SessionStore<WebPortalSession>> sessions);

      ~ComplianceWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      void Close();

    private:
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      Beam::IO::OpenState m_openState;

      Beam::WebServices::HttpResponse OnLoadDirectoryEntryComplianceRuleEntry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnAddComplianceRuleEntry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnUpdateComplianceRuleEntry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnDeleteComplianceRuleEntry(
        const Beam::WebServices::HttpRequest& request);
  };
}

#endif
