#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
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
    MetaRiskServlet<VirtualAdministrationClient*,
    std::unique_ptr<VirtualMarketDataClient>,
    std::unique_ptr<VirtualOrderExecutionClient>, TriggerTimer,
    std::shared_ptr<FixedTimeClient>, LocalRiskDataStore*>>;

  using TestInventoryMessage = RecordMessage<InventoryMessage,
    TestServiceProtocolClient>;

  struct Client {
    std::unique_ptr<VirtualServiceLocatorClient> m_serviceLocatorClient;
    std::unique_ptr<VirtualOrderExecutionClient> m_orderExecutionClient;
    std::unique_ptr<TestServiceProtocolClient> m_riskClient;
  };

  struct Fixture {
    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    UidServiceTestEnvironment m_uidServiceEnvironment;
    optional<AdministrationServiceTestEnvironment>
      m_administrationServiceEnvironment;
    optional<MarketDataServiceTestEnvironment> m_marketDataServiceEnvironment;
    MockOrderExecutionDriver m_driver;
    optional<OrderExecutionServiceTestEnvironment>
      m_orderExecutionServiceEnvironment;
    std::shared_ptr<Queue<DirectoryEntry>> m_accounts;
    std::shared_ptr<Queue<PrimitiveOrder*>> m_orders;
    LocalRiskDataStore m_dataStore;
    std::shared_ptr<TestServerConnection> m_serverConnection;
    optional<TestServletContainer> m_container;
    std::shared_ptr<VirtualAdministrationClient> m_administrationClient;

    Fixture()
        : m_accounts(std::make_shared<Queue<DirectoryEntry>>()),
          m_serverConnection(std::make_shared<TestServerConnection>()) {
      m_serviceLocatorEnvironment.Open();
      m_uidServiceEnvironment.Open();
      auto serviceLocatorClient =
        std::shared_ptr(m_serviceLocatorEnvironment.BuildClient());
      serviceLocatorClient->SetCredentials("root", "");
      serviceLocatorClient->Open();
      m_administrationServiceEnvironment.emplace(serviceLocatorClient);
      m_administrationServiceEnvironment->Open();
      m_administrationServiceEnvironment->MakeAdministrator(
        DirectoryEntry::GetRootAccount());
      m_administrationClient = std::shared_ptr(
        m_administrationServiceEnvironment->BuildClient(
        Ref(*serviceLocatorClient)));
      m_marketDataServiceEnvironment.emplace(serviceLocatorClient,
        m_administrationClient);
      m_marketDataServiceEnvironment->Open();
      m_orderExecutionServiceEnvironment.emplace(GetDefaultMarketDatabase(),
        GetDefaultDestinationDatabase(), serviceLocatorClient,
        m_uidServiceEnvironment.BuildClient(), m_administrationClient,
        MakeVirtualTimeClient(std::make_unique<FixedTimeClient>()),
        MakeVirtualOrderExecutionDriver(&m_driver));
      m_orders = std::make_shared<Queue<PrimitiveOrder*>>();
      m_driver.GetPublisher().Monitor(m_orders);
      m_orderExecutionServiceEnvironment->Open();
      auto exchangeRates = std::vector<ExchangeRate>();
      exchangeRates.push_back(ExchangeRate(CurrencyPair(
        DefaultCurrencies::USD(), DefaultCurrencies::CAD()), 1));
      m_container.emplace(Initialize(serviceLocatorClient,
        Initialize(m_accounts, &*m_administrationClient,
        m_marketDataServiceEnvironment->BuildClient(
        Ref(*serviceLocatorClient)),
        m_orderExecutionServiceEnvironment->BuildClient(
        Ref(*serviceLocatorClient)),
        [] {
          return std::make_unique<TriggerTimer>();
        }, std::make_shared<FixedTimeClient>(), &m_dataStore, exchangeRates,
        GetDefaultMarketDatabase(), GetDefaultDestinationDatabase())),
        m_serverConnection, factory<std::unique_ptr<TriggerTimer>>());
      m_container->Open();
      m_marketDataServiceEnvironment->Publish(TSLA, BboQuote(
        Quote(*Money::FromValue("1.00"), 100, Side::BID),
        Quote(*Money::FromValue("1.01"), 100, Side::ASK),
        second_clock::universal_time()));
      m_marketDataServiceEnvironment->Publish(XIU, BboQuote(
        Quote(*Money::FromValue("2.00"), 100, Side::BID),
        Quote(*Money::FromValue("2.01"), 100, Side::ASK),
        second_clock::universal_time()));
    }

    Client MakeClient(std::string name) {
      auto account = m_serviceLocatorEnvironment.GetRoot().MakeAccount(name,
        "1234", DirectoryEntry::GetStarDirectory());
      m_administrationClient->StoreRiskParameters(account,
        RiskParameters(DefaultCurrencies::USD(), 100000 * Money::ONE,
        RiskState::Type::ACTIVE, 100 * Money::ONE, 1, minutes(1)));
      m_administrationClient->StoreRiskState(account, RiskState::Type::ACTIVE);
      auto client = Client();
      client.m_serviceLocatorClient = m_serviceLocatorEnvironment.BuildClient();
      client.m_serviceLocatorClient->SetCredentials(name, "1234");
      client.m_serviceLocatorClient->Open();
      client.m_orderExecutionClient =
        m_orderExecutionServiceEnvironment->BuildClient(
        Ref(*client.m_serviceLocatorClient));
      client.m_orderExecutionClient->Open();
      client.m_riskClient = std::make_unique<TestServiceProtocolClient>(
        Initialize("test", Ref(*m_serverConnection)), Initialize());
      RegisterRiskServices(Store(client.m_riskClient->GetSlots()));
      RegisterRiskMessages(Store(client.m_riskClient->GetSlots()));
      client.m_riskClient->Open();
      auto authenticator = SessionAuthenticator(
        Ref(*client.m_serviceLocatorClient));
      authenticator(*client.m_riskClient);
      return client;
    }
  };
}

TEST_SUITE("RiskServlet") {
  TEST_CASE_FIXTURE(Fixture, "reset_region") {
    auto client = MakeClient("simba");
    m_administrationServiceEnvironment->MakeAdministrator(
      client.m_serviceLocatorClient->GetAccount());
    auto subscriptionResponse =
      client.m_riskClient->SendRequest<SubscribeRiskPortfolioUpdatesService>();
    m_accounts->Push(client.m_serviceLocatorClient->GetAccount());
    client.m_orderExecutionClient->Submit(OrderFields::BuildLimitOrder(XIU,
      Side::BID, 200, Money::ONE));
    auto& receivedBid = *m_orders->Pop();
    Accept(receivedBid);
    receivedBid.With(
      [&] (auto status, const auto& executionReports) {
        auto report = ExecutionReport::BuildUpdatedReport(
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
      client.m_serviceLocatorClient->GetAccount());
    auto bidInventory = bidMessage->GetRecord().inventories[0].inventory;
    REQUIRE(bidInventory.m_position.m_key ==
      RiskPosition::Key(XIU, DefaultCurrencies::CAD()));
    REQUIRE(bidInventory.m_position.m_quantity == 100);
    REQUIRE(bidInventory.m_position.m_costBasis == 100 * Money::ONE);
    REQUIRE(bidInventory.m_fees == 3 * Money::CENT);
    REQUIRE(bidInventory.m_grossProfitAndLoss == Money::ZERO);
    REQUIRE(bidInventory.m_transactionCount == 1);
    REQUIRE(bidInventory.m_volume == 100);
    client.m_orderExecutionClient->Submit(OrderFields::BuildLimitOrder(XIU,
      Side::ASK, 200, Money::ONE + Money::CENT));
    auto& receivedAsk = *m_orders->Pop();
    Accept(receivedAsk);
    receivedAsk.With(
      [&] (auto status, const auto& executionReports) {
        auto report = ExecutionReport::BuildUpdatedReport(
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
      client.m_serviceLocatorClient->GetAccount());
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
      client.m_serviceLocatorClient->GetAccount());
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
