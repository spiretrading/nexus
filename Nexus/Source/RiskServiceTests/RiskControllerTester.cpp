#include <Beam/Queues/ConverterQueueReader.hpp>
#include <Beam/Queues/FilteredQueueReader.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/RiskController.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::RiskService;

namespace {
  auto ACCOUNT = DirectoryEntry::MakeAccount(153, "simba");
  auto TSLA = Security("TSLA", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  auto XIU = Security("XIU", DefaultMarkets::TSX(), DefaultCountries::CA());

  struct Fixture {
    TestEnvironment m_environment;
    TestServiceClients m_serviceClients;

    Fixture()
        : m_serviceClients(Ref(m_environment)) {
      m_environment.Open();
      m_serviceClients.Open();
    }
  };
}

TEST_SUITE("RiskController") {
  TEST_CASE_FIXTURE(Fixture, "single_security") {
    auto exchangeRates = std::vector<ExchangeRate>();
    auto dataStore = LocalRiskDataStore();
    dataStore.Open();
    auto controller = RiskController(ACCOUNT,
      &m_serviceClients.GetAdministrationClient(),
      &m_serviceClients.GetMarketDataClient(),
      &m_serviceClients.GetOrderExecutionClient(),
      m_serviceClients.BuildTimer(seconds(1)),
      &m_serviceClients.GetTimeClient(), &dataStore, exchangeRates,
      GetDefaultMarketDatabase(), GetDefaultDestinationDatabase());
  }
}
