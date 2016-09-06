#ifndef NEXUS_CLIENTWEBPORTALSERVLET_HPP
#define NEXUS_CLIENTWEBPORTALSERVLET_HPP
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/AuthenticatedSession.hpp>
#include <Beam/WebServices/FileStore.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include <boost/noncopyable.hpp>
#include "ClientWebPortal/ClientWebPortal/ClientWebPortal.hpp"

namespace Nexus {
namespace ClientWebPortal {

  /*! \class ClientWebPortalServlet
      \brief Implements a web servlet for Spire client services.
   */
  class ClientWebPortalServlet : private boost::noncopyable {
    public:

      //! Constructs a ClientWebPortalServlet.
      /*!
        \param serviceClients The clients used to access Spire services.
      */
      ClientWebPortalServlet(Beam::RefType<ServiceClients> serviceClients);

      ~ClientWebPortalServlet();

      //! Returns the HTTP request slots.
      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      void Open();

      void Close();

    private:
      Beam::Serialization::JsonSender<Beam::IO::SharedBuffer> m_sender;
      Beam::WebServices::FileStore m_fileStore;
      Beam::WebServices::SessionStore<Beam::WebServices::AuthenticatedSession>
        m_sessions;
      ServiceClients* m_serviceClients;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      Beam::WebServices::HttpResponse OnIndex(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnServeFile(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadCurrentAccount(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnStorePassword(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLogin(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLogout(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnCreateAccount(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadTradingGroup(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadManagedTradingGroups(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadAccountRoles(
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
      Beam::WebServices::HttpResponse OnLoadCountryDatabase(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadCurrencyDatabase(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadMarketDatabase(
        const Beam::WebServices::HttpRequest& request);
  };
}
}

#endif
