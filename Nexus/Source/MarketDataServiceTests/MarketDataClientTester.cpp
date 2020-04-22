#include <Beam/Queues/Queue.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <boost/any.hpp>
#include <boost/optional/optional.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::Queries;

namespace {
  using TestMarketDataClient = MarketDataClient<
    TestServiceProtocolClientBuilder>;

  const auto SECURITY_A = Security("TST", DefaultMarkets::NYSE(),
    DefaultCountries::US());

  struct ClientEntry {
    TestMarketDataClient m_client;

    ClientEntry(const TestServiceProtocolClientBuilder& clientBuilder)
      : m_client(clientBuilder) {}
  };

  struct Fixture {
    std::shared_ptr<TestServerConnection> m_serverConnection;
    TestServiceProtocolServer m_server;
    std::shared_ptr<Queue<std::vector<any>>> m_requestQueue;

    Fixture()
      : m_serverConnection(std::make_shared<TestServerConnection>()),
        m_server(m_serverConnection, factory<std::unique_ptr<TriggerTimer>>(),
          NullSlot(), NullSlot()),
        m_requestQueue(std::make_shared<Queue<std::vector<any>>>()) {
      Nexus::Queries::RegisterQueryTypes(
        Store(m_server.GetSlots().GetRegistry()));
      RegisterMarketDataRegistryServices(Store(m_server.GetSlots()));
      RegisterMarketDataRegistryMessages(Store(m_server.GetSlots()));
      QueryBboQuotesService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind(&Fixture::OnQuerySecurityBboQuotes, this,
        std::placeholders::_1, std::placeholders::_2));
      m_server.Open();
    }

    template<typename T>
    using Request = RequestToken<
      TestServiceProtocolServer::ServiceProtocolClient, T>;

    std::unique_ptr<ClientEntry> MakeClient() {
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
            "test", Ref(*m_serverConnection));
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      auto clientEntry = std::make_unique<ClientEntry>(builder);
      clientEntry->m_client.Open();
      return clientEntry;
    }

    void OnQuerySecurityBboQuotes(Request<QueryBboQuotesService>& request,
        const SecurityMarketDataQuery& query) {
      auto parameters = std::vector<any>();
      parameters.push_back(request);
      parameters.push_back(query);
      m_requestQueue->Push(parameters);
    }
  };
}

TEST_SUITE("MarketDataClient") {
  TEST_CASE_FIXTURE(Fixture, "real_time_bbo_quote_query") {
    auto client = MakeClient();
    auto query = SecurityMarketDataQuery();
    query.SetIndex(SECURITY_A);
    query.SetRange(Beam::Queries::Range::RealTime());
    auto bboQuotes = std::make_shared<Queue<BboQuote>>();
    client->m_client.QueryBboQuotes(query, bboQuotes);
    auto request = m_requestQueue->Top();
    m_requestQueue->Pop();
    REQUIRE(request.size() == 2);
    auto requestToken = optional<Request<QueryBboQuotesService>>();
    REQUIRE_NOTHROW(requestToken =
      any_cast<Request<QueryBboQuotesService>>(request[0]));
    optional<SecurityMarketDataQuery> requestQuery;
    REQUIRE_NOTHROW(requestQuery =
      any_cast<SecurityMarketDataQuery>(request[1]));
    REQUIRE(requestQuery->GetIndex() == SECURITY_A);
    REQUIRE(requestQuery->GetRange() == Beam::Queries::Range::RealTime());
    auto queryResponse = BboQuoteQueryResult();
    queryResponse.m_queryId = 123;
    requestToken->SetResult(queryResponse);
    auto bbo = BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(Money::ONE + Money::CENT, 200, Side::ASK),
      second_clock::universal_time());
    SendRecordMessage<BboQuoteMessage>(requestToken->GetClient(),
      SequencedValue(IndexedValue(bbo, SECURITY_A),
      Beam::Queries::Sequence(1)));
    auto updatedBbo = bboQuotes->Top();
    REQUIRE(updatedBbo == bbo);
  }
}
