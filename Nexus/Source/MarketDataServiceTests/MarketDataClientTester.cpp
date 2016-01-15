#include "Nexus/MarketDataServiceTests/MarketDataClientTester.hpp"
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::Queries;
using namespace std;

namespace {
  Security SECURITY_A("TST", DefaultMarkets::NYSE(), DefaultCountries::US());
}

struct MarketDataClientTester::ClientEntry {
  TestMarketDataClient m_client;

  ClientEntry(const ServiceProtocolClientBuilder& clientBuilder);
};

MarketDataClientTester::ClientEntry::ClientEntry(
    const ServiceProtocolClientBuilder& clientBuilder)
    : m_client(clientBuilder) {}

void MarketDataClientTester::setUp() {
  m_serverConnection.Initialize();
  m_server.Initialize(&*m_serverConnection,
    factory<std::shared_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
  Nexus::Queries::RegisterQueryTypes(Store(m_server->GetSlots().GetRegistry()));
  RegisterMarketDataRegistryServices(Store(m_server->GetSlots()));
  RegisterMarketDataRegistryMessages(Store(m_server->GetSlots()));
  QueryBboQuotesService::AddRequestSlot(Store(m_server->GetSlots()),
    std::bind(&MarketDataClientTester::OnQuerySecurityBboQuotes, this,
    std::placeholders::_1, std::placeholders::_2));
  m_server->Open();
  m_requestQueue = std::make_shared<Queue<vector<any>>>();
}

void MarketDataClientTester::tearDown() {
  m_requestQueue.reset();
  m_server.Reset();
  m_serverConnection.Reset();
}

void MarketDataClientTester::TestRealTimeBboQuoteQuery() {
  std::unique_ptr<ClientEntry> client = MakeClient();
  SecurityMarketDataQuery query;
  query.SetIndex(SECURITY_A);
  query.SetRange(Beam::Queries::Range::RealTime());
  std::shared_ptr<Queue<BboQuote>> bboQuotes =
    std::make_shared<Queue<BboQuote>>();
  client->m_client.QueryBboQuotes(query, bboQuotes);
  vector<any> request = m_requestQueue->Top();
  m_requestQueue->Pop();
  CPPUNIT_ASSERT(request.size() == 2);
  optional<Request<QueryBboQuotesService>> requestToken;
  CPPUNIT_ASSERT_NO_THROW(requestToken =
    any_cast<Request<QueryBboQuotesService>>(request[0]));
  optional<SecurityMarketDataQuery> requestQuery;
  CPPUNIT_ASSERT_NO_THROW(requestQuery =
    any_cast<SecurityMarketDataQuery>(request[1]));
  CPPUNIT_ASSERT(requestQuery->GetIndex() == SECURITY_A);
  CPPUNIT_ASSERT(requestQuery->GetRange() == Beam::Queries::Range::RealTime());
  BboQuoteQueryResult queryResponse;
  queryResponse.m_queryId = 123;
  requestToken->SetResult(queryResponse);
  BboQuote bbo(Quote(Money::ONE, 100, Side::BID),
    Quote(Money::ONE + Money::CENT, 200, Side::ASK),
    second_clock::universal_time());
  SendRecordMessage<BboQuoteMessage>(requestToken->GetClient(),
    MakeSequencedValue(MakeIndexedValue(bbo, SECURITY_A),
    Beam::Queries::Sequence(1)));
  BboQuote updatedBbo = bboQuotes->Top();
  CPPUNIT_ASSERT(updatedBbo == bbo);
}

unique_ptr<MarketDataClientTester::ClientEntry> MarketDataClientTester::
    MakeClient() {
  ServiceProtocolClientBuilder builder(
    [=] {
      return std::make_unique<ServiceProtocolClientBuilder::Channel>(("test"),
        Ref(*m_serverConnection));
    },
    [] {
      return std::make_unique<ServiceProtocolClientBuilder::Timer>();
    });
  unique_ptr<ClientEntry> clientEntry = std::make_unique<ClientEntry>(builder);
  clientEntry->m_client.Open();
  return clientEntry;
}

void MarketDataClientTester::OnQuerySecurityBboQuotes(
    Request<QueryBboQuotesService>& request,
    const SecurityMarketDataQuery& query) {
  vector<any> parameters;
  parameters.push_back(request);
  parameters.push_back(query);
  m_requestQueue->Push(parameters);
}
