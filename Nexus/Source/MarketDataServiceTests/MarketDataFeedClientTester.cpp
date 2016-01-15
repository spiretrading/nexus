#include "Nexus/MarketDataServiceTests/MarketDataFeedClientTester.hpp"
#include <Beam/ServiceLocator/NullAuthenticator.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace std;

void MarketDataFeedClientTester::setUp() {
  m_serverConnection.Initialize();
  m_server.Initialize(&*m_serverConnection,
    factory<std::shared_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
  m_samplingTimer.Initialize();
  m_client.Initialize(Initialize(string("test"), Ref(*m_serverConnection)),
    NullAuthenticator(), &*m_samplingTimer, Initialize());
  m_server->Open();
  RegisterMarketDataFeedMessages(Store(m_server->GetSlots()));
  m_client->Open();
}

void MarketDataFeedClientTester::tearDown() {
  m_client.Reset();
  m_samplingTimer.Reset();
  m_server.Reset();
  m_serverConnection.Reset();
}

void MarketDataFeedClientTester::TestSettingBbo() {
  Async<void> sentMessages;
  Security security("TST", DefaultMarkets::NYSE(), DefaultCountries::US());
  SecurityBboQuote bbo(BboQuote(Quote(Money::CENT, 100, Side::BID),
    Quote(2 * Money::CENT, 100, Side::ASK), second_clock::universal_time()),
    security);
  AddMessageSlot<SendMarketDataFeedMessages>(Store(m_server->GetSlots()),
    [&] (ServiceProtocolServer::ServiceProtocolClient& client,
        const vector<MarketDataFeedMessage>& messages) {
      CPPUNIT_ASSERT(messages.size() == 1);
      CPPUNIT_ASSERT(messages.front().type() == typeid(SecurityBboQuote));
      SecurityBboQuote bboQuote = boost::get<SecurityBboQuote>(
        messages.front());
      CPPUNIT_ASSERT(bboQuote == bbo);
      sentMessages.GetEval().SetResult();
    });
  m_client->PublishBboQuote(bbo);
  m_samplingTimer->Trigger();
  sentMessages.Get();
}

void MarketDataFeedClientTester::TestPublishMarketOrderImbalance() {
#if 0
  Async<void> sentMessages;
  Security security("TST", DefaultMarkets::NYSE(), DefaultCountries::US());
  OrderImbalance orderImbalance(DefaultMarkets::NYSE(), security, Side::BID,
    100, Money::CENT, second_clock::universal_time());
  AddMessageSlot<SendMarketDataFeedMessages>(Store(m_server->GetSlots()),
    [&] (ServiceProtocolServer::ServiceProtocolClient& client,
        const vector<MarketDataFeedMessage>& messages) {
      CPPUNIT_ASSERT(messages.size() == 1);
      CPPUNIT_ASSERT(messages.front().type() == typeid(OrderImbalance));
      OrderImbalance message = boost::get<OrderImbalance>(messages.front());
      CPPUNIT_ASSERT(message == orderImbalance);
      sentMessages.GetEval().SetResult();
    });
  m_client->PublishOrderImbalance(orderImbalance);
  m_samplingTimer->Trigger();
  sentMessages.Get();
#endif
}
