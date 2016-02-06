#ifndef NEXUS_CLIENTWEBPORTALSERVLET_HPP
#define NEXUS_CLIENTWEBPORTALSERVLET_HPP
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <boost/noncopyable.hpp>
#include "ClientWebPortal/ApplicationTypes.hpp"

namespace Nexus {
namespace ClientWebPortal {
  class ClientWebPortalServlet : private boost::noncopyable {
    public:

      ClientWebPortalServlet(
        Beam::RefType<ApplicationServiceLocatorClient> serviceLocatorClient);

      ~ClientWebPortalServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots() const;

      void Open();

      void Close();

    private:
  };
}
}

#endif
