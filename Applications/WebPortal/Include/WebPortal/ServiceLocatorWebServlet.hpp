#ifndef NEXUS_SERVICE_LOCATOR_WEB_SERVLET_HPP
#define NEXUS_SERVICE_LOCATOR_WEB_SERVLET_HPP
#include <functional>
#include <Beam/IO/OpenState.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus {

  /** Provides a web interface to the ServiceLocator. */
  class ServiceLocatorWebServlet {
    public:

      /**
       * Type of function used to build session Clients.
       * @param username The username to login with.
       * @param password The username's password.
       */
      using ClientsBuilder = std::function<Clients (
        const std::string& username, const std::string& password)>;

      /**
       * Constructs a ServiceLocatorWebServlet.
       * @param sessions The available web sessions.
       * @param clients_builder The function used to build session Clients.
       */
      ServiceLocatorWebServlet(
        Beam::Ref<Beam::WebServices::SessionStore<WebPortalSession>> sessions,
        ClientsBuilder clients_builder);

      ~ServiceLocatorWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> get_slots();

      void close();

    private:
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      ClientsBuilder m_clients_builder;
      Beam::IO::OpenState m_open_state;

      ServiceLocatorWebServlet(const ServiceLocatorWebServlet&) = delete;
      ServiceLocatorWebServlet& operator=(
        const ServiceLocatorWebServlet&) = delete;
      Beam::WebServices::HttpResponse on_login(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_logout(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_current_account(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_load_directory_entry_from_id(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_store_password(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_search_directory_entry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_create_account(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse on_create_group(
        const Beam::WebServices::HttpRequest& request);
  };
}

#endif
