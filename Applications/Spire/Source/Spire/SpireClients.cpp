#include "Spire/Spire/SpireClients.hpp"
#include <stdexcept>
#include <Beam/IO/ConnectException.hpp>
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
#include "Spire/SignIn/SignInException.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  template<typename Box, typename Client>
  auto make_client(Ref<ApplicationServiceLocatorClient> service_locator_client,
      auto reason) {
    try {
      return Box(std::in_place_type<Client>, Ref(service_locator_client));
    } catch(const std::exception&) {
      throw SignInException(reason);
    }
  }
}

SpireClients::SpireClients(
  std::unique_ptr<ApplicationServiceLocatorClient> serviceLocatorClient)
BEAM_SUPPRESS_THIS_INITIALIZER()
  : m_application_service_locator_client(std::move(serviceLocatorClient)),
    m_service_locator_client(m_application_service_locator_client.get()),
    m_definitions_client([&] {
      auto definitions_client =
        make_client<DefinitionsClient, ApplicationDefinitionsClient>(
          Ref(*m_application_service_locator_client),
          "Definitions server is unavailable.");
      auto minimum_version =
        definitions_client.load_minimum_spire_client_version();
      if(std::stoi(minimum_version) > std::stoi(std::string(SPIRE_VERSION))) {
        BOOST_THROW_EXCEPTION(
          SignInException("Unsupported version, update to " + minimum_version));
      }
      return definitions_client;
    }()),
    m_administration_client(
      make_client<AdministrationClient, ApplicationAdministrationClient>(
        Ref(*m_application_service_locator_client),
        "Administration server is unavailable.")),
    m_market_data_client(
      make_client<MarketDataClient, ApplicationMarketDataClient>(
        Ref(*m_application_service_locator_client),
        "Market data server is unavailable.")),
    m_charting_client(make_client<ChartingClient, ApplicationChartingClient>(
      Ref(*m_application_service_locator_client),
      "Charting server is unavailable.")),
    m_compliance_client(
      make_client<ComplianceClient, ApplicationComplianceClient>(
        Ref(*m_application_service_locator_client),
        "Compliance server is unavailable.")),
    m_order_execution_client(
      make_client<OrderExecutionClient, ApplicationOrderExecutionClient>(
        Ref(*m_application_service_locator_client),
        "Order execution server is unavailable.")),
    m_risk_client(make_client<RiskClient, ApplicationRiskClient>(
      Ref(*m_application_service_locator_client),
      "Risk server is unavailable.")),
    m_time_client([&] {
      try {
        return make_live_ntp_time_client(m_service_locator_client);
      } catch(const std::exception&) {
        throw SignInException("Time server is unavailable.");
      }
    }()) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

SpireClients::~SpireClients() {
  close();
}

ServiceLocatorClient& SpireClients::get_service_locator_client() {
  return m_service_locator_client;
}

AdministrationClient& SpireClients::get_administration_client() {
  return m_administration_client;
}

DefinitionsClient& SpireClients::get_definitions_client() {
  return m_definitions_client;
}

ChartingClient& SpireClients::get_charting_client() {
  return m_charting_client;
}

ComplianceClient& SpireClients::get_compliance_client() {
  return m_compliance_client;
}

MarketDataClient& SpireClients::get_market_data_client() {
  return m_market_data_client;
}

OrderExecutionClient& SpireClients::get_order_execution_client() {
  return m_order_execution_client;
}

RiskClient& SpireClients::get_risk_client() {
  return m_risk_client;
}

TimeClient& SpireClients::get_time_client() {
  return m_time_client;
}

std::unique_ptr<Timer> SpireClients::make_timer(time_duration expiry) {
  return std::make_unique<Timer>(std::make_unique<LiveTimer>(expiry));
}

void SpireClients::close() {
  m_time_client.close();
  m_service_locator_client.close();
  m_risk_client.close();
  m_order_execution_client.close();
  m_market_data_client.close();
  m_compliance_client.close();
  m_charting_client.close();
  m_administration_client.close();
  m_definitions_client.close();
}
