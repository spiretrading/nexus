#include "Spire/Spire/SpireServiceClients.hpp"
#include <stdexcept>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/RegistryService/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/ChartingService/ApplicationDefinitions.hpp"
#include "Nexus/Compliance/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"
#include "Nexus/RiskService/ApplicationDefinitions.hpp"
#include "Spire/Login/LoginException.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::RegistryService;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::ChartingService;
using namespace Nexus::Compliance;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace Spire;

namespace {
  template<typename Box, typename Client>
  auto make_client(auto& service_locator_client, auto reason) {
    try {
      return Box(std::in_place_type<Client>, service_locator_client->Get());
    } catch(const std::exception&) {
      throw LoginException(reason);
    }
  }
}

SpireServiceClients::SpireServiceClients(
  std::unique_ptr<ApplicationServiceLocatorClient> serviceLocatorClient)
BEAM_SUPPRESS_THIS_INITIALIZER()
  : m_applicationServiceLocatorClient(std::move(serviceLocatorClient)),
    m_serviceLocatorClient(m_applicationServiceLocatorClient->Get()),
    m_definitionsClient([&] {
      auto definitionsClient =
        make_client<DefinitionsClientBox, ApplicationDefinitionsClient>(
          m_applicationServiceLocatorClient,
          "Definitions server is unavailable.");
      auto minimum_version = definitionsClient.LoadMinimumSpireClientVersion();
      if(std::stoi(minimum_version) > std::stoi(std::string(SPIRE_VERSION))) {
        BOOST_THROW_EXCEPTION(
          LoginException("Unsupported version, update to " + minimum_version));
      }
      return definitionsClient;
    }()),
    m_registryClient(make_client<RegistryClientBox, ApplicationRegistryClient>(
      m_applicationServiceLocatorClient, "Registry server is unavailable.")),
    m_administrationClient(make_client<
      AdministrationClientBox, ApplicationAdministrationClient>(
        m_applicationServiceLocatorClient,
        "Administration server is unavailable.")),
    m_marketDataClient(make_client<
      MarketDataClientBox, ApplicationMarketDataClient>(
        m_applicationServiceLocatorClient,
        "Market data server is unavailable.")),
    m_chartingClient(make_client<ChartingClientBox, ApplicationChartingClient>(
      m_applicationServiceLocatorClient, "Charting server is unavailable.")),
    m_complianceClient(make_client<
      ComplianceClientBox, ApplicationComplianceClient>(
        m_applicationServiceLocatorClient,
        "Compliance server is unavailable.")),
    m_orderExecutionClient(make_client<
      OrderExecutionClientBox, ApplicationOrderExecutionClient>(
        m_applicationServiceLocatorClient,
        "Order execution server is unavailable.")),
    m_riskClient(make_client<RiskClientBox, ApplicationRiskClient>(
      m_applicationServiceLocatorClient, "Risk server is unavailable.")),
    m_timeClient([&] {
      try {
        return MakeLiveNtpTimeClientFromServiceLocator(m_serviceLocatorClient);
      } catch(const std::exception&) {
        throw LoginException("Time server is unavailable.");
      }
    }()) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

SpireServiceClients::~SpireServiceClients() {
  Close();
}

ServiceLocatorClientBox& SpireServiceClients::GetServiceLocatorClient() {
  return m_serviceLocatorClient;
}

RegistryClientBox& SpireServiceClients::GetRegistryClient() {
  return m_registryClient;
}

AdministrationClientBox& SpireServiceClients::GetAdministrationClient() {
  return m_administrationClient;
}

DefinitionsClientBox& SpireServiceClients::GetDefinitionsClient() {
  return m_definitionsClient;
}

ChartingClientBox& SpireServiceClients::GetChartingClient() {
  return m_chartingClient;
}

ComplianceClientBox& SpireServiceClients::GetComplianceClient() {
  return m_complianceClient;
}

MarketDataClientBox& SpireServiceClients::GetMarketDataClient() {
  return m_marketDataClient;
}

OrderExecutionClientBox& SpireServiceClients::GetOrderExecutionClient() {
  return m_orderExecutionClient;
}

RiskClientBox& SpireServiceClients::GetRiskClient() {
  return m_riskClient;
}

TimeClientBox& SpireServiceClients::GetTimeClient() {
  return m_timeClient;
}

std::unique_ptr<TimerBox> SpireServiceClients::MakeTimer(time_duration expiry) {
  return std::make_unique<TimerBox>(std::make_unique<LiveTimer>(expiry));
}

void SpireServiceClients::Close() {
  m_timeClient.Close();
  m_serviceLocatorClient.Close();
  m_riskClient.Close();
  m_orderExecutionClient.Close();
  m_marketDataClient.Close();
  m_complianceClient.Close();
  m_chartingClient.Close();
  m_administrationClient.Close();
  m_registryClient.Close();
  m_definitionsClient.Close();
}
