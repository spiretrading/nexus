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
    return try_or_nest([&] {
      auto config = Configuration();
      config.m_interface = extract<IpAddress>(node, "interface");
      auto addresses = std::vector<IpAddress>();
      addresses.push_back(config.m_interface);
      config.m_addresses =
        extract<std::vector<IpAddress>>(node, "addresses", addresses);
      return config;
    }, std::runtime_error("Failed to parse configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv, "1.0-r" WEB_PORTAL_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_locator_client_config = try_or_nest([&] {
      return ServiceLocatorClientConfig::parse(
        get_node(config, "service_locator"));
    }, std::runtime_error("Error parsing section 'service_locator'."));
    auto clients = ServiceClients(service_locator_client_config.m_username,
      service_locator_client_config.m_password,
      service_locator_client_config.m_address);
    auto service_config = try_or_nest([&] {
      return Configuration::parse(get_node(config, "server"));
    }, std::runtime_error("Error parsing section 'server'."));
    auto clients_builder =
      [&] (const std::string& username, const std::string& password) {
        return Clients(std::in_place_type<ServiceClients>, username, password,
          service_locator_client_config.m_address);
      };
    auto server = WebPortalServletContainer(
      init(std::move(clients_builder), Clients(&clients)),
      init(service_config.m_interface));
    wait_for_kill_event();
    clients.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
