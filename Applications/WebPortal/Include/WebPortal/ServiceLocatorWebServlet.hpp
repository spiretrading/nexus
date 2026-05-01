#ifndef NEXUS_SERVICE_LOCATOR_WEB_SERVLET_HPP
#define NEXUS_SERVICE_LOCATOR_WEB_SERVLET_HPP
#include <functional>
#include <Beam/IO/OpenState.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/WebSessionStore.hpp>
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
      using ClientsBuilder = std::function<
        Clients (const std::string& username, const std::string& password)>;

      /**
       * Type of function used to build session Clients from a session.
       * @param session_id The encrypted session id.
       * @param key The encryption key used to encode the session id.
       */
      using SessionClientsBuilder = std::function<
        Clients (const std::string& session_id, unsigned int key)>;

      /**
       * Constructs a ServiceLocatorWebServlet.
       * @param sessions The available web sessions.
       * @param clients_builder The function used to build session Clients.
       * @param session_clients_builder The function used to build session
       *        Clients from an existing session.
       */
      ServiceLocatorWebServlet(
        Beam::Ref<Beam::WebSessionStore<WebPortalSession>> sessions,
        ClientsBuilder clients_builder,
        SessionClientsBuilder session_clients_builder);

      ~ServiceLocatorWebServlet();

      std::vector<Beam::HttpRequestSlot> get_slots();
      void close();

    private:
      Beam::WebSessionStore<WebPortalSession>* m_sessions;
      ClientsBuilder m_clients_builder;
      SessionClientsBuilder m_session_clients_builder;
      Beam::OpenState m_open_state;

      ServiceLocatorWebServlet(const ServiceLocatorWebServlet&) = delete;
      ServiceLocatorWebServlet& operator=(
        const ServiceLocatorWebServlet&) = delete;
      Beam::HttpResponse on_login(const Beam::HttpRequest& request);
      Beam::HttpResponse on_logout(const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_current_account(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_load_directory_entry_from_id(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_store_password(const Beam::HttpRequest& request);
      Beam::HttpResponse on_search_directory_entry(
        const Beam::HttpRequest& request);
      Beam::HttpResponse on_create_account(const Beam::HttpRequest& request);
      Beam::HttpResponse on_create_group(const Beam::HttpRequest& request);
      Beam::HttpResponse on_login_from_session(
        const Beam::HttpRequest& request);
  };
}

#endif
