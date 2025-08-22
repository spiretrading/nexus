#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;
using namespace Nexus::Tests;

namespace {
  struct Fixture {
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaMarketDataRegistryServlet<MarketDataRegistry*,
        LocalHistoricalDataStore*, AdministrationClient>, NativePointerPolicy>;
    FixedTimeClient m_time_client;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    optional<ServiceLocatorClientBox> m_servlet_service_locator_client;
    optional<AdministrationClient> m_servlet_administration_client;
    MarketDataRegistry m_registry;
    LocalHistoricalDataStore m_data_store;
    optional<ServletContainer::Servlet::Servlet> m_servlet;
    std::shared_ptr<TestServerConnection> m_server_connection;
    optional<ServletContainer> m_container;
    DirectoryEntry m_client_account;
    std::unique_ptr<TestServiceProtocolClient> m_client;

    auto make_account(const std::string& name, const DirectoryEntry& parent) {
      return m_service_locator_environment.GetRoot().MakeAccount(
        name, "", parent);
    }

    auto make_client(const std::string& name) {
      auto service_locator_client =
        m_service_locator_environment.MakeClient(name, "");
      auto authenticator = SessionAuthenticator(service_locator_client);
      auto protocol_client = std::make_unique<TestServiceProtocolClient>(
        Initialize(name, *m_server_connection), Initialize());
      Nexus::Queries::RegisterQueryTypes(
        Beam::Store(protocol_client->GetSlots().GetRegistry()));
      RegisterMarketDataRegistryServices(Store(protocol_client->GetSlots()));
      RegisterMarketDataRegistryMessages(Store(protocol_client->GetSlots()));
      authenticator(*protocol_client);
      return std::tuple(
        service_locator_client.GetAccount(), std::move(protocol_client));
    }

    Fixture()
        : m_time_client(time_from_string("2024-07-04 12:00:00")),
          m_server_connection(std::make_shared<TestServerConnection>()),
          m_administration_environment(
            make_administration_service_test_environment(
              m_service_locator_environment)),
          m_registry(DEFAULT_VENUES, get_default_time_zone_database()) {
      auto servlet_account = make_account(
        "market_data_service", DirectoryEntry::GetStarDirectory());
      m_administration_environment.make_administrator(servlet_account);
      m_service_locator_environment.GetRoot().StorePermissions(
        servlet_account, DirectoryEntry::GetStarDirectory(), Permissions(~0));
      m_servlet_service_locator_client =
        m_service_locator_environment.MakeClient(servlet_account.m_name, "");
      m_servlet_administration_client =
        m_administration_environment.make_client(
          *m_servlet_service_locator_client);
      m_servlet.emplace(
        *m_servlet_administration_client, &m_registry, &m_data_store);
      m_container.emplace(Initialize(
        *m_servlet_service_locator_client, &*m_servlet),
        m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
      m_client_account =
        make_account("client", DirectoryEntry::GetStarDirectory());
      auto global_entitlement = m_servlet_administration_client->
        load_entitlements().get_entries().front().m_group_entry;
      m_servlet_administration_client->store_entitlements(
        m_client_account, {global_entitlement});
      std::tie(m_client_account, m_client) = make_client("client");
    }
  };

  template<typename QueryService, typename MessageType,
    typename MakeData, typename LoadFromStore, typename GetRecord>
  void test_query_publish(Fixture& fixture, const Security& security,
      MakeData&& make_data, LoadFromStore&& load_from_store,
      GetRecord&& get_record) {
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Range::RealTime());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto result = fixture.m_client->SendRequest<QueryService>(query);
    REQUIRE(result.m_queryId != -1);
    REQUIRE(result.m_snapshot.empty());
    auto data = make_data();
    fixture.m_servlet->publish(data, 1);
    auto message = fixture.m_client->ReadMessage();
    auto received_message = std::dynamic_pointer_cast<
      RecordMessage<MessageType, TestServiceProtocolClient>>(message);
    auto received_data = get_record(received_message->GetRecord());
    REQUIRE(received_data->GetValue() == *data);
    auto data_store_query = SecurityMarketDataQuery();
    data_store_query.SetIndex(security);
    data_store_query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    data_store_query.SetRange(
      received_data.GetSequence(), Increment(received_data.GetSequence()));
    auto stored_data = load_from_store(data_store_query);
    REQUIRE(stored_data.size() == 1);
    REQUIRE(stored_data.front() == ToSequencedValue(received_data));
    SUBCASE("query_no_entitlement") {
      auto client_account =
        fixture.make_account("client2", DirectoryEntry::GetStarDirectory());
      auto client = std::unique_ptr<TestServiceProtocolClient>();
      std::tie(client_account, client) = fixture.make_client("client2");
      auto query = SecurityMarketDataQuery();
      query.SetIndex(security);
      query.SetRange(Range::Total());
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto result = client->SendRequest<QueryService>(query);
      REQUIRE(result.m_queryId == -1);
      REQUIRE(result.m_snapshot.empty());
    }
  }
}

TEST_SUITE("MarketDataRegistryServlet") {
  TEST_CASE("query_security_info") {
    auto fixture = Fixture();
    auto security = Security("A", TSX);
    auto info = SecurityInfo(security, "SECURITY A", "", 100);
    fixture.m_data_store.store(info);
    fixture.m_registry.add(info);
    auto query = make_security_info_query(security);
    auto result =
      fixture.m_client->SendRequest<QuerySecurityInfoService>(query);
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == info);
  }

  TEST_CASE("load_security_info_from_prefix") {
    auto fixture = Fixture();
    auto security_a = Security("A", TSX);
    auto info_a = SecurityInfo(security_a, "SECURITY A", "", 100);
    fixture.m_registry.add(info_a);
    auto security_b = Security("B", TSX);
    auto info_b = SecurityInfo(security_b, "SECURITY B", "", 100);
    fixture.m_registry.add(info_b);
    auto security_c = Security("C", TSX);
    auto info_c = SecurityInfo(security_c, "SECURITY C", "", 100);
    fixture.m_registry.add(info_c);
    auto result =
      fixture.m_client->SendRequest<LoadSecurityInfoFromPrefixService>("A");
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == info_a);
  }

  TEST_CASE("query_publish_order_imbalance") {
    auto fixture = Fixture();
    auto security = Security("A", TSX);
    auto info = SecurityInfo(security, "SECURITY A", "", 100);
    fixture.m_registry.add(info);
    auto query = VenueMarketDataQuery();
    query.SetIndex(TSX);
    query.SetRange(Range::RealTime());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto result =
      fixture.m_client->SendRequest<QueryOrderImbalancesService>(query);
    REQUIRE(result.m_queryId != -1);
    REQUIRE(result.m_snapshot.empty());
    auto imbalance = VenueOrderImbalance(
      OrderImbalance(security, Side::ASK, 100, Money::ONE,
        fixture.m_time_client.GetTime()), TSX);
    fixture.m_servlet->publish(imbalance, 1);
    auto message = fixture.m_client->ReadMessage();
    auto received_imbalance_message = std::dynamic_pointer_cast<
      RecordMessage<OrderImbalanceMessage, TestServiceProtocolClient>>(message);
    auto received_imbalance =
      received_imbalance_message->GetRecord().order_imbalance;
    REQUIRE(received_imbalance->GetValue() == *imbalance);
    auto data_store_query = VenueMarketDataQuery();
    data_store_query.SetIndex(TSX);
    data_store_query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    data_store_query.SetRange(received_imbalance.GetSequence(),
      Increment(received_imbalance.GetSequence()));
    auto stored_imbalances =
      fixture.m_data_store.load_order_imbalances(data_store_query);
    REQUIRE(stored_imbalances.size() == 1);
    REQUIRE(
      stored_imbalances.front() == ToSequencedValue(received_imbalance));
    SUBCASE("query_no_entitlement") {
      auto client_account =
        fixture.make_account("client2", DirectoryEntry::GetStarDirectory());
      auto client = std::unique_ptr<TestServiceProtocolClient>();
      std::tie(client_account, client) = fixture.make_client("client2");
      auto query = VenueMarketDataQuery();
      query.SetIndex(TSX);
      query.SetRange(Range::Total());
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto result = client->SendRequest<QueryOrderImbalancesService>(query);
      REQUIRE(result.m_queryId == -1);
      REQUIRE(result.m_snapshot.empty());
    }
  }

  TEST_CASE("query_publish_bbo_quote") {
    auto fixture = Fixture();
    auto security = Security("A", TSX);
    auto info = SecurityInfo(security, "SECURITY A", "", 100);
    fixture.m_registry.add(info);
    test_query_publish<QueryBboQuotesService, BboQuoteMessage>(
      fixture, security, [&] {
        return SecurityBboQuote(BboQuote(Quote(Money::CENT, 100, Side::BID),
          Quote(2 * Money::CENT, 200, Side::ASK),
          fixture.m_time_client.GetTime()), security);
      }, [&] (const auto& query) {
        return fixture.m_data_store.load_bbo_quotes(query);
      }, [] (const auto& record) {
        return record.bbo_quote;
      });
  }

  TEST_CASE("query_publish_book_quote") {
    auto fixture = Fixture();
    auto security = Security("A", TSX);
    auto info = SecurityInfo(security, "SECURITY A", "", 100);
    fixture.m_registry.add(info);
    test_query_publish<QueryBookQuotesService, BookQuoteMessage>(
      fixture, security, [&] {
        return SecurityBookQuote(BookQuote("MP1", false, TSX,
          Quote(Money::CENT, 100, Side::BID),
          fixture.m_time_client.GetTime()), security);
      }, [&] (const auto& query) {
        return fixture.m_data_store.load_book_quotes(query);
      }, [] (const auto& record) {
        return record.book_quote;
      });
  }

  TEST_CASE("query_publish_time_and_sale") {
    auto fixture = Fixture();
    auto security = Security("A", TSX);
    auto info = SecurityInfo(security, "SECURITY A", "", 100);
    fixture.m_registry.add(info);
    test_query_publish<QueryTimeAndSalesService, TimeAndSaleMessage>(
      fixture, security, [&] {
        return SecurityTimeAndSale(TimeAndSale(fixture.m_time_client.GetTime(),
          Money::ONE, 100, TimeAndSale::Condition(), "TSX", "", ""), security);
      }, [&] (const auto& query) {
        return fixture.m_data_store.load_time_and_sales(query);
      }, [] (const auto& record) {
        return record.time_and_sale;
      });
  }
}
