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
  template<typename T>
  struct ByPassPtr {
    using Type = GetDereferenceType<T>;

    std::unique_ptr<T> m_value;

    ByPassPtr(std::unique_ptr<T> value)
      : m_value(std::move(value)) {}

    ByPassPtr(ByPassPtr&& ptr)
      : m_value(std::move(ptr.m_value)) {}

    Type& operator *() const {
      return **m_value;
    }

    Type* operator ->() const {
      return &**m_value;
    }
  };
}

SpireServiceClients::SpireServiceClients(
  std::unique_ptr<ApplicationServiceLocatorClient> serviceLocatorClient)
BEAM_SUPPRESS_THIS_INITIALIZER()
  : m_applicationServiceLocatorClient(std::move(serviceLocatorClient)),
    m_serviceLocatorClient(m_applicationServiceLocatorClient->Get()),
    m_definitionsClient([&] {
      auto definitionsClient = DefinitionsClientBox(
        std::in_place_type<ApplicationDefinitionsClient>,
        m_applicationServiceLocatorClient->Get());
      auto minimumVersion = definitionsClient.LoadMinimumSpireClientVersion();
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
