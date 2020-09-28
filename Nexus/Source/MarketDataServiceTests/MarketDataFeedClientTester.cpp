#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServer.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/ServiceLocator/NullAuthenticator.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;

namespace {
  struct Fixture {
    using TestServiceProtocolServer = ServiceProtocolServer<
      std::shared_ptr<TestServerConnection>, BinarySender<SharedBuffer>,
      NullEncoder, std::unique_ptr<TriggerTimer>>;
    using TestMarketDataFeedClient = MarketDataFeedClient<std::string,
      TriggerTimer*, MessageProtocol<TestClientChannel,
      BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;

    boost::optional<TestServiceProtocolServer> m_server;
    Beam::Threading::TriggerTimer m_samplingTimer;
    boost::optional<TestMarketDataFeedClient> m_client;

    Fixture() {
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_server.emplace(serverConnection,
        factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
      m_client.emplace(Initialize("test", *serverConnection),
        NullAuthenticator(), &m_samplingTimer, Initialize());
      RegisterMarketDataFeedMessages(Store(m_server->GetSlots()));
    }
  };
}

TEST_SUITE("MarketDataFeedClient") {
  TEST_CASE_FIXTURE(Fixture, "setting_bbo") {
    auto sentMessages = Async<void>();
    auto security = Security("TST", DefaultMarkets::NYSE(),
      DefaultCountries::US());
    auto bbo = SecurityBboQuote(BboQuote(Quote(Money::CENT, 100, Side::BID),
      Quote(2 * Money::CENT, 100, Side::ASK), second_clock::universal_time()),
      security);
    AddMessageSlot<SendMarketDataFeedMessages>(Store(m_server->GetSlots()),
      [&] (auto& client, auto& messages) {
        REQUIRE(messages.size() == 1);
        REQUIRE(messages.front().type() == typeid(SecurityBboQuote));
        auto bboQuote = boost::get<SecurityBboQuote>(messages.front());
        REQUIRE(bboQuote == bbo);
        sentMessages.GetEval().SetResult();
      });
    m_client->PublishBboQuote(bbo);
    m_samplingTimer.Trigger();
    sentMessages.Get();
  }

  TEST_CASE_FIXTURE(Fixture, "publish_market_order_imbalance") {
    auto sentMessages = Async<void>();
    auto security = Security("TST", DefaultMarkets::NYSE(),
      DefaultCountries::US());
    auto orderImbalance = MarketOrderImbalance(OrderImbalance(security,
      Side::BID, 100, Money::CENT, second_clock::universal_time()),
      DefaultMarkets::NYSE());
    AddMessageSlot<SendMarketDataFeedMessages>(Store(m_server->GetSlots()),
      [&] (auto& client, auto& messages) {
        REQUIRE(messages.size() == 1);
        REQUIRE(messages.front().type() == typeid(MarketOrderImbalance));
        auto message = boost::get<MarketOrderImbalance>(messages.front());
        REQUIRE(message == orderImbalance);
        sentMessages.GetEval().SetResult();
      });
    m_client->PublishOrderImbalance(orderImbalance);
    m_samplingTimer.Trigger();
    sentMessages.Get();
  }
}
