#ifndef NEXUS_COMPLIANCEWEBSERVLET_HPP
#define NEXUS_COMPLIANCEWEBSERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include <boost/noncopyable.hpp>
#include "web_portal/web_portal/web_portal.hpp"
#include "web_portal/web_portal/web_portal_session.hpp"
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"

namespace Nexus {
namespace ClientWebPortal {

  /*! \class ComplianceWebServlet
      \brief Provides a web interface to the ComplianceService.
   */
  class ComplianceWebServlet : private boost::noncopyable {
    public:

      //! Constructs a ComplianceWebServlet.
      /*!
        \param sessions The available web sessions.
        \param serviceClients The clients used to access Spire services.
      */
      ComplianceWebServlet(Beam::RefType<
        Beam::WebServices::SessionStore<ClientWebPortalSession>> sessions,
        Beam::RefType<ApplicationServiceClients> serviceClients);

      ~ComplianceWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      void Open();

      void Close();

    private:
      ApplicationServiceClients* m_serviceClients;
      Beam::WebServices::SessionStore<ClientWebPortalSession>* m_sessions;
      Beam::IO::OpenState m_openState;

      void Shutdown();
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
}

#endif
