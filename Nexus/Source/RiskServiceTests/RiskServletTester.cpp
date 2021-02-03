#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/RiskServlet.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::UidService;
using namespace Beam::UidService::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::RiskService;

namespace {
  auto TSLA = Security("TSLA", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  auto XIU = Security("XIU", DefaultMarkets::TSX(), DefaultCountries::CA());

  using TestServletContainer = TestAuthenticatedServiceProtocolServletContainer<
    MetaRiskServlet<AdministrationClientBox, MarketDataClientBox,
      OrderExecutionClientBox, TriggerTimer, std::shared_ptr<FixedTimeClient>,
      LocalRiskDataStore*>>;

  using TestInventoryMessage = RecordMessage<InventoryMessage,
    TestServiceProtocolClient>;

  struct Client {
    ServiceLocatorClientBox m_serviceLocatorClient;
    optional<OrderExecutionClientBox> m_orderExecutionClient;
    std::unique_ptr<TestServiceProtocolClient> m_riskClient;

    Client(ServiceLocatorClientBox serviceLocatorClient)
      : m_serviceLocatorClient(std::move(serviceLocatorClient)) {}
  };

  struct Fixture {
    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    UidServiceTestEnvironment m_uidServiceEnvironment;
    AdministrationServiceTestEnvironment m_administrationEnvironment;
    std::shared_ptr<Queue<DirectoryEntry>> m_accounts;
    MarketDataServiceTestEnvironment m_marketDataServiceEnvironment;
    MockOrderExecutionDriver m_driver;
    OrderExecutionServiceTestEnvironment m_orderExecutionServiceEnvironment;
    std::shared_ptr<Queue<PrimitiveOrder*>> m_orders;
    LocalRiskDataStore m_dataStore;
    std::shared_ptr<TestServerConnection> m_serverConnection;
    optional<TestServletContainer> m_container;

    Fixture()
        : m_administrationEnvironment(MakeAdministrationServiceTestEnvironment(
            m_serviceLocatorEnvironment)),
          m_accounts(std::make_shared<Queue<DirectoryEntry>>()),
          m_marketDataServiceEnvironment(m_serviceLocatorEnvironment.GetRoot(),
            m_administrationEnvironment.GetClient()),
          m_orderExecutionServiceEnvironment(GetDefaultMarketDatabase(),
            GetDefaultDestinationDatabase(),
            m_serviceLocatorEnvironment.GetRoot(),
            m_uidServiceEnvironment.MakeClient(),
            m_administrationEnvironment.GetClient(),
            TimeClientBox(std::in_place_type<FixedTimeClient>,
              time_from_string("2020-05-12 04:12:18")),
            MakeVirtualOrderExecutionDriver(&m_driver)),
          m_serverConnection(std::make_shared<TestServerConnection>()),
          m_orders(std::make_shared<Queue<PrimitiveOrder*>>()) {
      m_administrationEnvironment.MakeAdministrator(
        m_serviceLocatorEnvironment.GetRoot().GetAccount());
      m_driver.GetPublisher().Monitor(m_orders);
      auto exchangeRates = std::vector<ExchangeRate>();
      exchangeRates.push_back(ExchangeRate(CurrencyPair(
        DefaultCurrencies::USD(), DefaultCurrencies::CAD()), 1));
      m_container.emplace(Initialize(m_serviceLocatorEnvironment.GetRoot(),
        Initialize(m_accounts, m_administrationEnvironment.GetClient(),
          m_marketDataServiceEnvironment.MakeRegistryClient(
            m_serviceLocatorEnvironment.GetRoot()),
          m_orderExecutionServiceEnvironment.MakeClient(
            m_serviceLocatorEnvironment.GetRoot()),
          factory<std::unique_ptr<TriggerTimer>>(),
          std::make_shared<FixedTimeClient>(
            ptime(date(2020, 5, 12), time_duration(4, 12, 18))), &m_dataStore,
          exchangeRates, GetDefaultMarketDatabase(),
          GetDefaultDestinationDatabase())), m_serverConnection,
        factory<std::unique_ptr<TriggerTimer>>());
      m_marketDataServiceEnvironment.GetFeedClient().Publish(
        SecurityBboQuote(BboQuote(
          Quote(*Money::FromValue("1.00"), 100, Side::BID),
          Quote(*Money::FromValue("1.01"), 100, Side::ASK),
          second_clock::universal_time()), TSLA));
      m_marketDataServiceEnvironment.GetFeedClient().Publish(
        SecurityBboQuote(BboQuote(
          Quote(*Money::FromValue("2.00"), 100, Side::BID),
          Quote(*Money::FromValue("2.01"), 100, Side::ASK),
          second_clock::universal_time()), XIU));
    }

    Client MakeClient(std::string name) {
      auto account = m_serviceLocatorEnvironment.GetRoot().MakeAccount(name,
        "1234", DirectoryEntry::GetStarDirectory());
      m_administrationEnvironment.GetClient().StoreRiskParameters(account,
        RiskParameters(DefaultCurrencies::USD(), 100000 * Money::ONE,
          RiskState::Type::ACTIVE, 100 * Money::ONE, 1, minutes(1)));
      m_administrationEnvironment.GetClient().StoreRiskState(account,
        RiskState::Type::ACTIVE);
      auto client = Client(m_serviceLocatorEnvironment.MakeClient(
        name, "1234"));
      client.m_orderExecutionClient.emplace(
        m_orderExecutionServiceEnvironment.MakeClient(
          client.m_serviceLocatorClient));
      client.m_riskClient = std::make_unique<TestServiceProtocolClient>(
        Initialize("test", *m_serverConnection), Initialize());
      RegisterRiskServices(Store(client.m_riskClient->GetSlots()));
      RegisterRiskMessages(Store(client.m_riskClient->GetSlots()));
      auto authenticator = SessionAuthenticator(client.m_serviceLocatorClient);
      authenticator(*client.m_riskClient);
      return client;
    }
  };
}

TEST_SUITE("RiskServlet") {
  TEST_CASE_FIXTURE(Fixture, "reset_region") {
    auto client = MakeClient("simba");
    m_administrationEnvironment.MakeAdministrator(
      client.m_serviceLocatorClient.GetAccount());
    auto subscriptionResponse =
      client.m_riskClient->SendRequest<SubscribeRiskPortfolioUpdatesService>();
    m_accounts->Push(client.m_serviceLocatorClient.GetAccount());
    client.m_orderExecutionClient->Submit(OrderFields::MakeLimitOrder(XIU,
      Side::BID, 200, Money::ONE));
    auto& receivedBid = *m_orders->Pop();
    Accept(receivedBid);
    receivedBid.With([&] (auto status, const auto& executionReports) {
      auto report = ExecutionReport::MakeUpdatedReport(
        executionReports.back(), OrderStatus::PARTIALLY_FILLED,
        executionReports.back().m_timestamp);
      report.m_executionFee = Money::CENT;
      report.m_processingFee = Money::CENT;
      report.m_commission = Money::CENT;
      report.m_lastQuantity = 100;
      report.m_lastPrice = Money::ONE;
      receivedBid.Update(report);
    });
    auto bidMessage = std::static_pointer_cast<TestInventoryMessage>(
      client.m_riskClient->ReadMessage());
    REQUIRE(bidMessage != nullptr);
    REQUIRE(bidMessage->GetRecord().inventories.size() == 1);
    REQUIRE(bidMessage->GetRecord().inventories[0].account ==
      client.m_serviceLocatorClient.GetAccount());
    auto bidInventory = bidMessage->GetRecord().inventories[0].inventory;
    REQUIRE(bidInventory.m_position.m_key ==
      RiskPosition::Key(XIU, DefaultCurrencies::CAD()));
    REQUIRE(bidInventory.m_position.m_quantity == 100);
    REQUIRE(bidInventory.m_position.m_costBasis == 100 * Money::ONE);
    REQUIRE(bidInventory.m_fees == 3 * Money::CENT);
    REQUIRE(bidInventory.m_grossProfitAndLoss == Money::ZERO);
    REQUIRE(bidInventory.m_transactionCount == 1);
    REQUIRE(bidInventory.m_volume == 100);
    client.m_orderExecutionClient->Submit(OrderFields::MakeLimitOrder(XIU,
      Side::ASK, 200, Money::ONE + Money::CENT));
    auto& receivedAsk = *m_orders->Pop();
    Accept(receivedAsk);
    receivedAsk.With([&] (auto status, const auto& executionReports) {
      auto report = ExecutionReport::MakeUpdatedReport(
        executionReports.back(), OrderStatus::FILLED,
        executionReports.back().m_timestamp);
      report.m_executionFee = Money::CENT;
      report.m_processingFee = Money::CENT;
      report.m_commission = Money::CENT;
      report.m_lastQuantity = 200;
      report.m_lastPrice = Money::ONE + Money::CENT;
      receivedAsk.Update(report);
    });
    auto askMessage = std::static_pointer_cast<TestInventoryMessage>(
      client.m_riskClient->ReadMessage());
    REQUIRE(askMessage != nullptr);
    REQUIRE(askMessage->GetRecord().inventories.size() == 1);
    REQUIRE(askMessage->GetRecord().inventories[0].account ==
      client.m_serviceLocatorClient.GetAccount());
    auto askInventory = askMessage->GetRecord().inventories[0].inventory;
    REQUIRE(askInventory.m_position.m_key ==
      RiskPosition::Key(XIU, DefaultCurrencies::CAD()));
    REQUIRE(askInventory.m_position.m_quantity == -100);
    REQUIRE(askInventory.m_position.m_costBasis ==
      -100 * (Money::ONE + Money::CENT));
    REQUIRE(askInventory.m_fees == 6 * Money::CENT);
    REQUIRE(askInventory.m_grossProfitAndLoss == Money::ONE);
    REQUIRE(askInventory.m_transactionCount == 2);
    REQUIRE(askInventory.m_volume == 300);
    client.m_riskClient->SendRequest<ResetRegionService>(XIU);
    auto resetMessage = std::static_pointer_cast<TestInventoryMessage>(
      client.m_riskClient->ReadMessage());
    REQUIRE(resetMessage != nullptr);
    REQUIRE(resetMessage->GetRecord().inventories.size() == 1);
    REQUIRE(resetMessage->GetRecord().inventories[0].account ==
      client.m_serviceLocatorClient.GetAccount());
    auto resetInventory = resetMessage->GetRecord().inventories[0].inventory;
    REQUIRE(resetInventory.m_position.m_key ==
      RiskPosition::Key(XIU, DefaultCurrencies::CAD()));
    REQUIRE(resetInventory.m_position.m_quantity == -100);
    REQUIRE(resetInventory.m_position.m_costBasis ==
      -100 * (Money::ONE + Money::CENT));
    REQUIRE(resetInventory.m_fees == Money::ZERO);
    REQUIRE(resetInventory.m_grossProfitAndLoss == Money::ZERO);
    REQUIRE(resetInventory.m_transactionCount == 0);
    REQUIRE(resetInventory.m_volume == 0);
  }
}
