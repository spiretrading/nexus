#include <Beam/Queues/Queue.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <boost/any.hpp>
#include <boost/optional/optional.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/ServiceMarketDataClient.hpp"

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
using namespace Nexus::DefaultVenues;

namespace {
  using TestMarketDataClient =
    ServiceMarketDataClient<TestServiceProtocolClientBuilder>;

  const auto SECURITY_A = Security("TST", NYSE);

  struct ClientEntry {
    TestMarketDataClient m_client;

    ClientEntry(const TestServiceProtocolClientBuilder& clientBuilder)
      : m_client(clientBuilder) {}
  };

  struct Fixture {
    std::shared_ptr<TestServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    std::shared_ptr<Queue<std::vector<any>>> m_request_queue;

    Fixture()
      : m_server_connection(std::make_shared<TestServerConnection>()),
        m_server(m_server_connection, factory<std::unique_ptr<TriggerTimer>>(),
          NullSlot(), NullSlot()),
        m_request_queue(std::make_shared<Queue<std::vector<any>>>()) {
      Nexus::RegisterQueryTypes(Store(m_server.GetSlots().GetRegistry()));
      RegisterMarketDataRegistryServices(Store(m_server.GetSlots()));
      RegisterMarketDataRegistryMessages(Store(m_server.GetSlots()));
      QueryBboQuotesService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_query_security_bbo_quotes, this));
    }

    template<typename T>
    using Request =
      RequestToken<TestServiceProtocolServer::ServiceProtocolClient, T>;

    std::unique_ptr<ClientEntry> make_client() {
      auto builder = TestServiceProtocolClientBuilder([this] {
        return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection);
      }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      return std::make_unique<ClientEntry>(builder);
    }

    void on_query_security_bbo_quotes(Request<QueryBboQuotesService>& request,
        const SecurityMarketDataQuery& query) {
      auto parameters = std::vector<any>();
      parameters.push_back(request);
      parameters.push_back(query);
      m_request_queue->Push(parameters);
    }
  };
}

TEST_SUITE("MarketDataClient") {
  TEST_CASE_FIXTURE(Fixture, "real_time_bbo_quote_query") {
    auto client = make_client();
    auto query = SecurityMarketDataQuery();
    query.SetIndex(SECURITY_A);
    query.SetRange(Beam::Queries::Range::RealTime());
    auto bbo_quotes = std::make_shared<Queue<BboQuote>>();
    client->m_client.query(query, bbo_quotes);
    auto request = m_request_queue->Pop();
    REQUIRE(request.size() == 2);
    auto request_token = optional<Request<QueryBboQuotesService>>();
    REQUIRE_NOTHROW(request_token =
      any_cast<Request<QueryBboQuotesService>>(request[0]));
    auto request_query = optional<SecurityMarketDataQuery>();
    REQUIRE_NOTHROW(
      request_query = any_cast<SecurityMarketDataQuery>(request[1]));
    REQUIRE(request_query->GetIndex() == SECURITY_A);
    REQUIRE(request_query->GetRange() == Beam::Queries::Range::RealTime());
    auto query_response = BboQuoteQueryResult();
    query_response.m_queryId = 123;
    request_token->SetResult(query_response);
    auto bbo = BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(Money::ONE + Money::CENT, 200, Side::ASK),
      time_from_string("2021-01-11 15:30:05.000"));
    SendRecordMessage<BboQuoteMessage>(request_token->GetClient(),
      SequencedValue(IndexedValue(bbo, SECURITY_A),
      Beam::Queries::Sequence(1)));
    auto updated_bbo = bbo_quotes->Pop();
    REQUIRE(updated_bbo == bbo);
  }
}
