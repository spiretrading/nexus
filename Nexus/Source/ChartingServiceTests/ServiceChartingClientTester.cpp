#include <memory>
#include <unordered_map>
#include <Beam/Queues/Queue.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/ChartingService/ServiceChartingClient.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;
using namespace Nexus::ChartingService;
using namespace Nexus::TechnicalAnalysis;

namespace {
  struct Fixture {
    using TestServiceChartingClient =
      ServiceChartingClient<TestServiceProtocolClientBuilder>;
    std::shared_ptr<TestServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    std::unique_ptr<TestServiceChartingClient> m_client;
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_handlers;

    Fixture()
        : m_server_connection(std::make_shared<TestServerConnection>()),
          m_server(m_server_connection,
            factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot()) {
      RegisterChartingServices(Store(m_server.GetSlots()));
      RegisterChartingMessages(Store(m_server.GetSlots()));
      auto builder = TestServiceProtocolClientBuilder([=] {
        return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection);
      }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_client = std::make_unique<TestServiceChartingClient>(builder);
    }

    template<typename T, typename F>
    void on_request(F&& handler) {
      using Slot = typename Services::Details::GetSlotType<RequestToken<
        TestServiceProtocolServer::ServiceProtocolClient, T>>::type;
      auto& stored_handler = m_handlers[typeid(T)];
      if(stored_handler) {
        *std::static_pointer_cast<Slot>(stored_handler) =
          std::forward<F>(handler);
      } else {
        auto shared_handler = std::make_shared<Slot>(std::forward<F>(handler));
        stored_handler = shared_handler;
        T::AddRequestSlot(Store(m_server.GetSlots()),
          [handler = std::move(shared_handler)] (auto&&... args) {
            try {
              (*handler)(std::forward<decltype(args)>(args)...);
            } catch(...) {
              throw ServiceRequestException("Test failed.");
            }
          });
      }
    }
  };
}

TEST_SUITE("ServiceChartingClient") {
  TEST_CASE("load_time_price_series") {
    auto fixture = Fixture();
    auto security = Security("TST", TSX);
    auto start = from_iso_string("20240131T100000");
    auto end = from_iso_string("20240131T110000");
    auto interval = time_duration(0, 1, 0);
    auto result = TimePriceQueryResult();
    result.start = Beam::Queries::Sequence(543);
    result.end = Beam::Queries::Sequence(654);
    result.series.push_back(TimePriceCandlestick(
      from_iso_string("20240131T100000"), from_iso_string("20240131T100100"),
      10 * Money::ONE, 11 * Money::ONE, 12 * Money::ONE, 9 * Money::ONE));
    fixture.on_request<LoadSecurityTimePriceSeriesService>(
      [&] (auto& request, const auto& received_security,
          const auto& received_start, const auto& received_end,
          const auto& received_interval) {
        REQUIRE(received_security == security);
        REQUIRE(received_start == start);
        REQUIRE(received_end == end);
        REQUIRE(received_interval == interval);
        request.SetResult(result);
      });
    auto received_result =
      fixture.m_client->load_time_price_series(security, start, end, interval);
    REQUIRE(result.series == received_result.series);
    REQUIRE(result.start == received_result.start);
    REQUIRE(result.end == received_result.end);
  }
}
