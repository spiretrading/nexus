#include <Beam/Queues/Queue.hpp>
#include <Beam/ServicesTests/ServiceClientFixture.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/ServiceMarketDataClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

namespace {
  const auto SECURITY_A = Security("TST", TSX);

  struct Fixture : ServiceClientFixture {
    using TestMarketDataClient =
      ServiceMarketDataClient<TestServiceProtocolClientBuilder>;
    std::unique_ptr<TestMarketDataClient> m_client;

    Fixture() {
      Nexus::register_query_types(out(m_server.get_slots().get_registry()));
      register_market_data_registry_services(out(m_server.get_slots()));
      register_market_data_registry_messages(out(m_server.get_slots()));
      m_client = make_client<TestMarketDataClient>();
    }
  };
}

TEST_SUITE("ServiceMarketDataClient") {
  TEST_CASE("real_time_bbo_quote_query") {
    auto fixture = Fixture();
    auto query = SecurityMarketDataQuery();
    query.set_index(SECURITY_A);
    query.set_range(Range::REAL_TIME);
    auto bbo_quotes = std::make_shared<Queue<BboQuote>>();
    auto bbo = BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 200),
      time_from_string("2021-01-11 15:30:05.000"));
    fixture.on_request<QueryBboQuotesService>(
      [&] (auto& request, const auto& query) {
        REQUIRE(query.get_index() == SECURITY_A);
        REQUIRE(query.get_range() == Range::REAL_TIME);
        auto response = BboQuoteQueryResult();
        response.m_id = 123;
        request.set(response);
        send_record_message<BboQuoteMessage>(request.get_client(),
          SequencedValue(IndexedValue(bbo, SECURITY_A), Beam::Sequence(1)));
      });
    fixture.m_client->query(query, bbo_quotes);
    auto updated_bbo = bbo_quotes->pop();
    REQUIRE(updated_bbo == bbo);
  }
}
