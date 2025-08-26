#ifndef NEXUS_ADMINISTRATION_WEB_SERVLET_HPP
#define NEXUS_ADMINISTRATION_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus {

  /** Provides a web interface to the AdministrationService. */
  class AdministrationWebServlet {
    public:

      /**
       * Constructs an AdministrationWebServlet.
       * @param sessions The available web sessions.
       */
      explicit AdministrationWebServlet(
        Beam::Ref<Beam::WebServices::SessionStore<WebPortalSession>> sessions);

      ~AdministrationWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> get_slots();

      void close();

    private:
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      Beam::IO::OpenState m_open_state;

      AdministrationWebServlet(const AdministrationWebServlet&) = delete;
      AdministrationWebServlet& operator=(
        const AdministrationWebServlet&) = delete;
      Beam::WebServices::HttpResponse on_load_accounts_by_roles(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_administrators_root_entry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_services_root_entry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_trading_groups_root_entry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_trading_group(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_managed_trading_groups(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_parent_trading_group(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_account_roles(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_store_account_roles(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_account_identity(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_store_account_identity(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_entitlements_database(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_account_entitlements(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_store_account_entitlements(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_risk_parameters(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_store_risk_parameters(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_account_modification_request(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_account_modification_request_ids(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse
        on_load_managed_account_modification_request_ids(
          const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_entitlement_modification(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse
        on_submit_entitlement_modification_request(
          const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_risk_modification(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_submit_risk_modification_request(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse
        on_load_account_modification_request_status(
          const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_approve_account_modification_request(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_reject_account_modification_request(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_message(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_message_ids(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse
        on_send_account_modification_request_message(
          const Beam::WebServices::HttpRequest& request);
  };
}

#endif
