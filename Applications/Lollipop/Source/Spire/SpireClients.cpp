#include "Spire/Spire/SpireClients.hpp"
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
#include "Version.hpp"

using namespace Beam;
using namespace Beam::RegistryService;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

SpireClients::SpireClients(
  std::unique_ptr<ApplicationServiceLocatorClient> serviceLocatorClient)
BEAM_SUPPRESS_THIS_INITIALIZER()
  : m_applicationServiceLocatorClient(std::move(serviceLocatorClient)),
    m_serviceLocatorClient(m_applicationServiceLocatorClient->Get()),
    m_definitionsClient([&] {
      auto definitionsClient = DefinitionsClient(
        std::in_place_type<ApplicationDefinitionsClient>,
        m_applicationServiceLocatorClient->Get());
      auto minimumVersion =
        definitionsClient.load_minimum_spire_client_version();
      if(std::stoi(minimumVersion) > std::stoi(std::string(SPIRE_VERSION))) {
        BOOST_THROW_EXCEPTION(std::runtime_error(
          ("Spire version incompatible.\n"
          "Minimum version required: ") + minimumVersion + ("\n"
          "Current version installed: ") + std::string(SPIRE_VERSION)));
      }
      return definitionsClient;
    }()),
    m_registryClient(std::in_place_type<ApplicationRegistryClient>,
      m_applicationServiceLocatorClient->Get()),
    m_administrationClient(std::in_place_type<ApplicationAdministrationClient>,
     m_applicationServiceLocatorClient->Get()),
    m_marketDataClient(std::in_place_type<ApplicationMarketDataClient>,
      m_applicationServiceLocatorClient->Get()),
    m_chartingClient(std::in_place_type<ApplicationChartingClient>,
      m_applicationServiceLocatorClient->Get()),
    m_complianceClient(std::in_place_type<ApplicationComplianceClient>,
      m_applicationServiceLocatorClient->Get()),
    m_orderExecutionClient(std::in_place_type<ApplicationOrderExecutionClient>,
      m_applicationServiceLocatorClient->Get()),
    m_riskClient(std::in_place_type<ApplicationRiskClient>,
      m_applicationServiceLocatorClient->Get()),
    m_timeClient(MakeLiveNtpTimeClientFromServiceLocator(
      m_serviceLocatorClient)) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

SpireClients::~SpireClients() {
  close();
}

ServiceLocatorClientBox& SpireClients::get_service_locator_client() {
  return m_serviceLocatorClient;
}

RegistryClientBox& SpireClients::get_registry_client() {
  return m_registryClient;
}

AdministrationClient& SpireClients::get_administration_client() {
  return m_administrationClient;
}

DefinitionsClient& SpireClients::get_definitions_client() {
  return m_definitionsClient;
}

ChartingClient& SpireClients::get_charting_client() {
  return m_chartingClient;
}

ComplianceClient& SpireClients::get_compliance_client() {
  return m_complianceClient;
}

MarketDataClient& SpireClients::get_market_data_client() {
  return m_marketDataClient;
}

OrderExecutionClient& SpireClients::get_order_execution_client() {
  return m_orderExecutionClient;
}

RiskClient& SpireClients::get_risk_client() {
  return m_riskClient;
}

TimeClientBox& SpireClients::get_time_client() {
  return m_timeClient;
}

std::unique_ptr<TimerBox> SpireClients::make_timer(time_duration expiry) {
  return std::make_unique<TimerBox>(std::make_unique<LiveTimer>(expiry));
}

void SpireClients::close() {
  m_timeClient.Close();
  m_serviceLocatorClient.Close();
  m_riskClient.close();
  m_orderExecutionClient.close();
  m_marketDataClient.close();
  m_complianceClient.close();
  m_chartingClient.close();
  m_administrationClient.close();
  m_registryClient.Close();
  m_definitionsClient.close();
}
