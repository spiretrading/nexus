#ifndef NEXUS_ADMINISTRATIONWEBSERVLET_HPP
#define NEXUS_ADMINISTRATIONWEBSERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include <boost/noncopyable.hpp>
#include "ClientWebPortal/ClientWebPortal/ClientWebPortal.hpp"
#include "ClientWebPortal/ClientWebPortal/ClientWebPortalSession.hpp"
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"

namespace Nexus {
namespace ClientWebPortal {

  /*! \class AdministrationWebServlet
      \brief Provides a web interface to the AdministrationService.
   */
  class AdministrationWebServlet : private boost::noncopyable {
    public:

      //! Constructs an AdministrationWebServlet.
      /*!
        \param sessions The available web sessions.
        \param serviceClients The clients used to access Spire services.
      */
      AdministrationWebServlet(Beam::RefType<
        Beam::WebServices::SessionStore<ClientWebPortalSession>> sessions,
        Beam::RefType<ApplicationServiceClients> serviceClients);

      ~AdministrationWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      void Open();

      void Close();

    private:
      ApplicationServiceClients* m_serviceClients;
      Beam::WebServices::SessionStore<ClientWebPortalSession>* m_sessions;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      Beam::WebServices::HttpResponse OnLoadOrganizationName(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadAccountsByRoles(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadTradingGroup(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadManagedTradingGroups(
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
}

#endif
