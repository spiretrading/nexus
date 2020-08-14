#include <Beam/Queues/ConverterQueueReader.hpp>
#include <Beam/Queues/FilteredQueueReader.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <doctest/doctest.h>
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
  auto ACCOUNT_A = DirectoryEntry::MakeAccount(153, "simba");
  auto ACCOUNT_B = DirectoryEntry::MakeAccount(155, "mufassa");
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
  TEST_CASE_FIXTURE(Fixture, "single_account") {
    auto accountUpdateQueue = std::make_shared<Queue<AccountUpdate>>();
    m_serviceClients.GetServiceLocatorClient().MonitorAccounts(
      accountUpdateQueue);
    auto accountQueue = MakeConverterQueueReader(MakeFilteredQueueReader(
      std::move(accountUpdateQueue),
      [] (const auto& update) {
        return update.m_type == AccountUpdate::Type::ADDED;
      }),
      [] (const auto& update) {
        return update.m_account;
      });
    auto exchangeRates = std::vector<ExchangeRate>();
    auto controller = RiskController(accountQueue,
      &m_serviceClients.GetAdministrationClient(),
      &m_serviceClients.GetMarketDataClient(),
      &m_serviceClients.GetOrderExecutionClient(),
      m_serviceClients.BuildTimer(seconds(1)),
      &m_serviceClients.GetTimeClient(), exchangeRates,
      GetDefaultDestinationDatabase());
  }
}
