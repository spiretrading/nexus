#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <Beam/WebServices/HttpServletContainer.hpp>
#include "Nexus/Clients/ServiceClients.hpp"
#include "WebPortal/WebPortalServlet.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::WebServices;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using WebPortalServletContainer =
    HttpServletContainer<WebPortalServlet, TcpServerSocket>;

  struct Configuration {
    IpAddress m_interface;
    std::vector<IpAddress> m_addresses;

    static Configuration parse(const YAML::Node& config);
  };

  Configuration Configuration::parse(const YAML::Node& node) {
    return TryOrNest([&] {
      auto config = Configuration();
      config.m_interface = Extract<IpAddress>(node, "interface");
      auto addresses = std::vector<IpAddress>();
      addresses.push_back(config.m_interface);
      config.m_addresses =
        Extract<std::vector<IpAddress>>(node, "addresses", addresses);
      return config;
    }, std::runtime_error("Failed to parse configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv, "0.9-r" WEB_PORTAL_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto service_locator_client_config = TryOrNest([&] {
      return ServiceLocatorClientConfig::Parse(
        GetNode(config, "service_locator"));
    }, std::runtime_error("Error parsing section 'service_locator'."));
    auto clients = ServiceClients(service_locator_client_config.m_username,
      service_locator_client_config.m_password,
      service_locator_client_config.m_address);
    auto service_config = TryOrNest([&] {
      return Configuration::parse(GetNode(config, "server"));
    }, std::runtime_error("Error parsing section 'server'."));
    auto clients_builder =
      [&] (const std::string& username, const std::string& password) {
        return Clients(std::in_place_type<ServiceClients>, username, password,
          service_locator_client_config.m_address);
      };
    auto server = WebPortalServletContainer(
      Initialize(std::move(clients_builder), Clients(&clients)),
      Initialize(service_config.m_interface));
    WaitForKillEvent();
    clients.close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
