#ifndef NEXUS_CLIENTWEBPORTALSERVLET_HPP
#define NEXUS_CLIENTWEBPORTALSERVLET_HPP
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
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
      ServiceClients* m_serviceClients;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      Beam::WebServices::HttpServerResponse OnLogin(
        const Beam::WebServices::HttpServerRequest& request);
  };
}
}

#endif
