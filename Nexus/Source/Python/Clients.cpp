#include "Nexus/Python/Clients.hpp"
#include "Nexus/Clients/ServiceClients.hpp"
#include "Nexus/Python/ToPythonClients.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_service_clients(module& module) {
  export_clients<ToPythonClients<Clients>>(module, "Clients");
  export_clients<ToPythonClients<ServiceClients>>(module, "ServiceClients").
    def(init([] (std::string username, std::string password, std::string host,
        unsigned short port) {
      return std::make_shared<ToPythonClients<ServiceClients>>(
        std::move(username), std::move(password),
        IpAddress(std::move(host), port));
    }));
}
