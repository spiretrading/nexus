#ifndef NEXUS_SERVICE_LOCATOR_WEB_SERVLET_HPP
#define NEXUS_SERVICE_LOCATOR_WEB_SERVLET_HPP
#include <functional>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include <boost/noncopyable.hpp>
#include "WebPortal/WebPortal.hpp"
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus::WebPortal {

  /** Provides a web interface to the ServiceLocator. */
  class ServiceLocatorWebServlet : private boost::noncopyable {
    public:

      /**
       * Type of function used to build session ServiceClients.
       * @param username The username to login with.
       * @param password The username's password.
       */
      using ServiceClientsBuilder = std::function<
        std::unique_ptr<VirtualServiceClients> (
        const std::string& username, const std::string& password)>;

      /**
       * Constructs a ServiceLocatorWebServlet.
       * @param sessions The available web sessions.
       * @param serviceClientsBuilder The function used to build session
       *        ServiceClients.
       */
      ServiceLocatorWebServlet(Beam::Ref<
        Beam::WebServices::SessionStore<WebPortalSession>> sessions,
        ServiceClientsBuilder serviceClientsBuilder);

      ~ServiceLocatorWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      void Close();

    private:
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      ServiceClientsBuilder m_serviceClientsBuilder;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      Beam::WebServices::HttpResponse OnLogin(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLogout(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadCurrentAccount(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadDirectoryEntryFromId(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnStorePassword(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnSearchDirectoryEntry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnCreateAccount(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnCreateGroup(
        const Beam::WebServices::HttpRequest& request);
  };
}

#endif
