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
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::RiskService;

namespace {
  auto TSLA = Security("TSLA", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  auto XIU = Security("XIU", DefaultMarkets::TSX(), DefaultCountries::CA());

  struct Fixture {
    TestEnvironment m_environment;
    TestServiceClients m_adminClients;
    TestServiceClients m_userClients;
    std::shared_ptr<Queue<const Order*>> m_orders;
    DirectoryEntry m_account;

    Fixture()
        : m_adminClients(Ref(m_environment)),
          m_userClients(Ref(m_environment)),
          m_orders(std::make_shared<Queue<const Order*>>()) {
      m_environment.MonitorOrderSubmissions(m_orders);
      m_environment.Open();
      m_environment.GetAdministrationEnvironment().MakeAdministrator(
        DirectoryEntry::GetRootAccount());
      m_environment.Publish(TSLA, BboQuote(
        Quote(*Money::FromValue("1.00"), 100, Side::BID),
        Quote(*Money::FromValue("1.01"), 100, Side::ASK),
        m_environment.GetTimeEnvironment().GetTime()));
      m_adminClients.Open();
      m_account = m_adminClients.GetServiceLocatorClient().MakeAccount("simba",
        "1234", DirectoryEntry::GetStarDirectory());
      m_adminClients.GetAdministrationClient().StoreRiskParameters(m_account,
        RiskParameters(DefaultCurrencies::USD(), 100000 * Money::ONE,
        RiskState::Type::ACTIVE, 2 * Money::ONE, 1, minutes(10)));
      m_adminClients.GetAdministrationClient().StoreRiskState(m_account,
        RiskState::Type::ACTIVE);
      m_userClients.GetServiceLocatorClient().SetCredentials("simba", "1234");
      m_userClients.Open();
    }
  };
}

TEST_SUITE("RiskController") {
  TEST_CASE_FIXTURE(Fixture, "single_security") {
    auto exchangeRates = std::vector<ExchangeRate>();
    auto dataStore = LocalRiskDataStore();
    dataStore.Open();
    auto controller = RiskController(m_account,
      &m_adminClients.GetAdministrationClient(),
      &m_adminClients.GetMarketDataClient(),
      &m_adminClients.GetOrderExecutionClient(),
      m_adminClients.BuildTimer(seconds(1)),
      &m_adminClients.GetTimeClient(), &dataStore, exchangeRates,
      GetDefaultMarketDatabase(), GetDefaultDestinationDatabase());
    auto state = std::make_shared<Queue<RiskState>>();
    controller.GetRiskStatePublisher().Monitor(state);
    REQUIRE(state->Pop() == RiskState::Type::ACTIVE);
    auto portfolio = std::make_shared<Queue<RiskPortfolio::UpdateEntry>>();
    controller.GetPortfolioPublisher().Monitor(portfolio);
    auto& order = m_userClients.GetOrderExecutionClient().Submit(
      OrderFields::BuildMarketOrder(TSLA, Side::BID, 100));
    auto receivedOrder = m_orders->Pop();
    m_environment.AcceptOrder(*receivedOrder);
    m_environment.FillOrder(*receivedOrder, *Money::FromValue("1.01"), 100);
    auto update = portfolio->Pop();
    REQUIRE(update.m_unrealizedSecurity == -Money::ONE);
    REQUIRE(update.m_unrealizedCurrency == -Money::ONE);
    m_environment.Publish(TSLA, BboQuote(
      Quote(*Money::FromValue("0.99"), 100, Side::BID),
      Quote(*Money::FromValue("1.00"), 100, Side::ASK),
      m_environment.GetTimeEnvironment().GetTime()));
    REQUIRE(state->Pop().m_type == RiskState::Type::CLOSE_ORDERS);
  }
}
