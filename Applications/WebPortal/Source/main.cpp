#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <Beam/WebServices/HttpServletContainer.hpp>
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"
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
using namespace Nexus::WebPortal;

namespace {
  using WebPortalServletContainer = HttpServletContainer<WebPortalServlet,
    TcpServerSocket>;

  struct Configuration {
    IpAddress m_interface;
    std::vector<IpAddress> m_addresses;

    static Configuration Parse(const YAML::Node& config);
  };

  Configuration Configuration::Parse(const YAML::Node& node) {
    return TryOrNest([&] {
      auto config = Configuration();
      config.m_interface = Extract<IpAddress>(node, "interface");
      auto addresses = std::vector<IpAddress>();
      addresses.push_back(config.m_interface);
      config.m_addresses = Extract<std::vector<IpAddress>>(node, "addresses",
        addresses);
      return config;
    }, std::runtime_error("Failed to parse configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv, "0.9-r" WEB_PORTAL_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto serviceLocatorClientConfig = TryOrNest([&] {
      return ServiceLocatorClientConfig::Parse(
        GetNode(config, "service_locator"));
    }, std::runtime_error("Error parsing section 'service_locator'."));
    auto serviceClients = MakeVirtualServiceClients(
      std::make_unique<ApplicationServiceClients>(
      serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password,
      serviceLocatorClientConfig.m_address));
    auto serviceConfig = TryOrNest([&] {
      return Configuration::Parse(GetNode(config, "server"));
    }, std::runtime_error("Error parsing section 'server'."));
    auto serviceClientsBuilder =
      [&] (const std::string& username, const std::string& password) {
        return MakeVirtualServiceClients(
          std::make_unique<ApplicationServiceClients>(username, password,
          serviceLocatorClientConfig.m_address));
      };
    auto server = WebPortalServletContainer(Initialize(
      std::move(serviceClientsBuilder), Ref(*serviceClients)),
      Initialize(serviceConfig.m_interface));
    WaitForKillEvent();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
