#include <Beam/Queues/Queue.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/RiskService/ConsolidatedRiskController.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskServiceTests/TestRiskDataStore.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::RiskService;
using namespace Nexus::RiskService::Tests;

namespace {
  auto TSLA = Security("TSLA", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  auto XIU = Security("XIU", DefaultMarkets::TSX(), DefaultCountries::CA());

  struct Fixture {
    TestEnvironment m_environment;
    TestServiceClients m_adminClients;
    TestServiceClients m_userAClients;
    TestServiceClients m_userBClients;
    std::shared_ptr<Queue<const Order*>> m_orders;
    DirectoryEntry m_accountA;
    DirectoryEntry m_accountB;

    Fixture()
        : m_adminClients(Ref(m_environment)),
          m_userAClients(Ref(m_environment)),
          m_userBClients(Ref(m_environment)),
          m_orders(std::make_shared<Queue<const Order*>>()) {
      m_environment.MonitorOrderSubmissions(m_orders);
      m_environment.GetAdministrationEnvironment().MakeAdministrator(
        DirectoryEntry::GetRootAccount());
      m_environment.Publish(TSLA, BboQuote(
        Quote(*Money::FromValue("1.00"), 100, Side::BID),
        Quote(*Money::FromValue("1.01"), 100, Side::ASK),
        m_environment.GetTimeEnvironment().GetTime()));
      m_accountA = MakeAccount("simba1", 2 * Money::ONE, minutes(10));
      m_accountB = MakeAccount("simba2", 10 * Money::ONE, minutes(3));
    }

    DirectoryEntry MakeAccount(std::string name, Money netLoss,
        time_duration transitionTime) {
      auto account = m_adminClients.GetServiceLocatorClient().MakeAccount(name,
        "1234", DirectoryEntry::GetStarDirectory());
      m_adminClients.GetAdministrationClient().StoreRiskParameters(account,
        RiskParameters(DefaultCurrencies::USD(), 100000 * Money::ONE,
          RiskState::Type::ACTIVE, netLoss, 1, transitionTime));
      m_adminClients.GetAdministrationClient().StoreRiskState(account,
        RiskState::Type::ACTIVE);
      return account;
    }
  };
}

TEST_SUITE("ConsolidatedRiskController") {
  TEST_CASE_FIXTURE(Fixture, "single_account") {
    auto exchangeRates = std::vector<ExchangeRate>();
    auto dataStore = LocalRiskDataStore();
    auto accounts = std::make_shared<Queue<DirectoryEntry>>();
    auto controller = ConsolidatedRiskController(accounts,
      &m_adminClients.GetAdministrationClient(),
      &m_adminClients.GetMarketDataClient(),
      &m_adminClients.GetOrderExecutionClient(),
      [=] {
        return m_adminClients.BuildTimer(seconds(1));
      },
      &m_adminClients.GetTimeClient(), &dataStore, exchangeRates,
      GetDefaultMarketDatabase(), GetDefaultDestinationDatabase());
    accounts->Push(m_accountA);
  }

  TEST_CASE_FIXTURE(Fixture, "data_store_exception") {
    auto exchangeRates = std::vector<ExchangeRate>();
    auto dataStore = TestRiskDataStore();
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestRiskDataStore::Operation>>>();
    dataStore.GetPublisher().Monitor(operations);
    auto accounts = std::make_shared<Queue<DirectoryEntry>>();
    auto controller = ConsolidatedRiskController(accounts,
      &m_adminClients.GetAdministrationClient(),
      &m_adminClients.GetMarketDataClient(),
      &m_adminClients.GetOrderExecutionClient(),
      [=] {
        return m_adminClients.BuildTimer(seconds(1));
      },
      &m_adminClients.GetTimeClient(), &dataStore, exchangeRates,
      GetDefaultMarketDatabase(), GetDefaultDestinationDatabase());
    auto states = std::make_shared<Queue<RiskStateEntry>>();
    controller.GetRiskStatePublisher().Monitor(states);
    accounts->Push(m_accountA);
    auto operation = operations->Pop();
    auto loadOperation = get<TestRiskDataStore::LoadInventorySnapshotOperation>(
      &*operation);
    REQUIRE(loadOperation);
    REQUIRE(*loadOperation->m_account == m_accountA);
    loadOperation->m_result.SetException(std::runtime_error("Fail"));
    auto state = states->Pop();
    REQUIRE(state.m_key == m_accountA);
    REQUIRE(state.m_value == RiskState::Type::DISABLED);
  }
}
