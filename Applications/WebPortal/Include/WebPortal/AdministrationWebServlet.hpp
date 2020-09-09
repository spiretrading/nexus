#ifndef NEXUS_ADMINISTRATION_WEB_SERVLET_HPP
#define NEXUS_ADMINISTRATION_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include <boost/noncopyable.hpp>
#include "WebPortal/WebPortal.hpp"
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus::WebPortal {

  /** Provides a web interface to the AdministrationService. */
  class AdministrationWebServlet : private boost::noncopyable {
    public:

      /**
       * Constructs an AdministrationWebServlet.
       * @param sessions The available web sessions.
       */
      explicit AdministrationWebServlet(Beam::Ref<
        Beam::WebServices::SessionStore<WebPortalSession>> sessions);

      ~AdministrationWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      void Close();

    private:
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      Beam::WebServices::HttpResponse OnLoadAccountsByRoles(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadAdministratorsRootEntry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadServicesRootEntry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadTradingGroupsRootEntry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadTradingGroup(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadManagedTradingGroups(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadParentTradingGroup(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadAccountRoles(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnStoreAccountRoles(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadAccountIdentity(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnStoreAccountIdentity(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadEntitlementsDatabase(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadAccountEntitlements(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnStoreAccountEntitlements(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadRiskParameters(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnStoreRiskParameters(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadAccountModificationRequest(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadAccountModificationRequestIds(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse
        OnLoadManagedAccountModificationRequestIds(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadEntitlementModification(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnSubmitEntitlementModificationRequest(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadRiskModification(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnSubmitRiskModificationRequest(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadAccountModificationRequestStatus(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnApproveAccountModificationRequest(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnRejectAccountModificationRequest(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadMessage(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadMessageIds(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnSendAccountModificationRequestMessage(
        const Beam::WebServices::HttpRequest& request);
  };
}

#endif
