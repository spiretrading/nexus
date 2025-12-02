#include "Nexus/Python/Clients.hpp"
#include "Nexus/Clients/ServiceClients.hpp"
#include "Nexus/Python/ToPythonClients.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  auto clients = std::unique_ptr<class_<Clients>>();
}

class_<Clients>& Nexus::Python::get_exported_clients() {
  return *clients;
}

void Nexus::Python::export_service_clients(module& module) {
  clients = std::make_unique<class_<Clients>>(
    export_clients<Clients>(module, "Clients"));
  export_clients<ToPythonClients<ServiceClients>>(module, "ServiceClients").
    def(init([] (std::string username, std::string password, std::string host,
        unsigned short port) {
      return std::make_unique<ToPythonClients<ServiceClients>>(
        std::move(username), std::move(password),
        IpAddress(std::move(host), port));
    }));
}
