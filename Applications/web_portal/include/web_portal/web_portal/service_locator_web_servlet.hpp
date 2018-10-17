#ifndef NEXUS_SERVICE_LOCATOR_WEB_SERVLET_HPP
#define NEXUS_SERVICE_LOCATOR_WEB_SERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"
#include "web_portal/web_portal/web_portal.hpp"
#include "web_portal/web_portal/web_portal_session.hpp"

namespace Nexus::WebPortal {

  //! Provides a web interface to the ServiceLocator.
  class ServiceLocatorWebServlet : private boost::noncopyable {
    public:

      //! Constructs a ServiceLocatorWebServlet.
      /*!
        \param sessions The available web sessions.
        \param serviceClients The clients used to access Spire services.
      */
      ServiceLocatorWebServlet(Beam::Ref<
        Beam::WebServices::SessionStore<WebPortalSession>> sessions,
        Beam::Ref<ApplicationServiceClients> serviceClients);

      ~ServiceLocatorWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      void Open();

      void Close();

    private:
      ApplicationServiceClients* m_serviceClients;
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      Beam::WebServices::HttpResponse OnLogin(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLogout(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadCurrentAccount(
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
