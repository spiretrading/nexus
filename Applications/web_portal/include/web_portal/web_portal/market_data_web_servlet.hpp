#ifndef NEXUS_MARKETDATAWEBSERVLET_HPP
#define NEXUS_MARKETDATAWEBSERVLET_HPP
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

  /*! \class MarketDataWebServlet
      \brief Provides a web interface to the MarketDataService.
   */
  class MarketDataWebServlet : private boost::noncopyable {
    public:

      //! Constructs a MarketDataWebServlet.
      /*!
        \param sessions The available web sessions.
        \param serviceClients The clients used to access Spire services.
      */
      MarketDataWebServlet(Beam::RefType<
        Beam::WebServices::SessionStore<ClientWebPortalSession>> sessions,
        Beam::RefType<ApplicationServiceClients> serviceClients);

      ~MarketDataWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      void Open();

      void Close();

    private:
      ApplicationServiceClients* m_serviceClients;
      Beam::WebServices::SessionStore<ClientWebPortalSession>* m_sessions;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      Beam::WebServices::HttpResponse OnLoadSecurityInfoFromPrefix(
        const Beam::WebServices::HttpRequest& request);
  };
}
}

#endif
