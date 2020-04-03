#include <fstream>
#include <iostream>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/TimerThreadPool.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <Beam/WebServices/HttpServletContainer.hpp>
#include <tclap/CmdLine.h>
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
using namespace TCLAP;

namespace {
  using WebPortalServletContainer = HttpServletContainer<WebPortalServlet,
    TcpServerSocket>;

  struct ServerConnectionInitializer {
    IpAddress m_interface;
    std::vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void ServerConnectionInitializer::Initialize(const YAML::Node& config) {
    m_interface = Extract<IpAddress>(config, "interface");
    auto addresses = std::vector<IpAddress>();
    addresses.push_back(m_interface);
    m_addresses = Extract<std::vector<IpAddress>>(config, "addresses",
      addresses);
  }
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "0.9-r" WEB_PORTAL_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto configArg = ValueArg<std::string>("c", "config", "Configuration file",
      false, "config.yml", "path");
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() <<
      std::endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  auto socketThreadPool = SocketThreadPool();
  auto timerThreadPool = TimerThreadPool();
  auto serviceLocatorClientConfig = ServiceLocatorClientConfig();
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      GetNode(config, "service_locator"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'service_locator': " << e.what() <<
      std::endl;
    return -1;
  }
  auto serviceClients = MakeVirtualServiceClients(
    std::make_unique<ApplicationServiceClients>(
    serviceLocatorClientConfig.m_address, serviceLocatorClientConfig.m_username,
    serviceLocatorClientConfig.m_password, Ref(socketThreadPool),
    Ref(timerThreadPool)));
  try {
    serviceClients->Open();
  } catch(const std::exception& e) {
    std::cerr << "Error logging in: " << e.what() << std::endl;
    return -1;
  }
  auto serverConnectionInitializer = ServerConnectionInitializer();
  try {
    serverConnectionInitializer.Initialize(GetNode(config, "server"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'server': " << e.what() << std::endl;
    return -1;
  }
  auto serviceClientsBuilder =
    [&] (const std::string& username, const std::string& password) {
      return MakeVirtualServiceClients(
        std::make_unique<ApplicationServiceClients>(
        serviceLocatorClientConfig.m_address, username, password,
        Ref(socketThreadPool), Ref(timerThreadPool)));
    };
  auto server = WebPortalServletContainer(Initialize(
    std::move(serviceClientsBuilder), Ref(*serviceClients)),
    Initialize(serverConnectionInitializer.m_interface, Ref(socketThreadPool)));
  try {
    server.Open();
  } catch(const std::exception& e) {
    std::cerr << "Error opening server: " << e.what() << std::endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
