#include <memory>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/ServiceOrderExecutionClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
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
using namespace Nexus::DefaultVenues;
using namespace Nexus::OrderExecutionService;

namespace {
  auto make_order_fields() {
    return make_limit_order_fields(
      Security("TST", TSX), Side::BID, "TSX", Quantity(100), Money::ONE);
  }

  struct Fixture {
    using TestServiceOrderExecutionClient =
      ServiceOrderExecutionClient<TestServiceProtocolClientBuilder>;
    std::shared_ptr<TestServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    std::unique_ptr<TestServiceOrderExecutionClient> m_client;
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_handlers;

    Fixture()
        : m_server_connection(std::make_shared<TestServerConnection>()),
          m_server(m_server_connection,
            factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot()) {
      Nexus::Queries::RegisterQueryTypes(
        Beam::Store(m_server.GetSlots().GetRegistry()));
      RegisterOrderExecutionServices(Store(m_server.GetSlots()));
      RegisterOrderExecutionMessages(Store(m_server.GetSlots()));
      auto builder = TestServiceProtocolClientBuilder([=, this] {
        return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection);
      }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_client = std::make_unique<TestServiceOrderExecutionClient>(builder);
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

    template<typename T, typename F>
    void on_message(F&& handler) {
      using Slot = typename Beam::Services::Details::RecordMessageSlot<
        RecordMessage<
          T, TestServiceProtocolServer::ServiceProtocolClient>>::Slot;
      auto& stored_handler = m_handlers[typeid(T)];
      if(stored_handler) {
        *std::static_pointer_cast<Slot>(stored_handler) =
          std::forward<F>(handler);
      } else {
        auto shared_handler = std::make_shared<Slot>(std::forward<F>(handler));
        stored_handler = shared_handler;
        AddMessageSlot<T>(Store(m_server.GetSlots()),
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

#define REQUIRE_NO_THROW(expression) \
  [&] { \
    REQUIRE_NOTHROW(return (expression)); \
    throw 0; \
  }()

TEST_SUITE("ServiceOrderExecutionClient") {
  TEST_CASE("load_order") {
    auto fixture = Fixture();
    auto id = 123;
    auto timestamp1 = time_from_string("2024-05-21 00:00:10.000");
    auto fields = make_order_fields();
    auto info = OrderInfo(fields, id, timestamp1);
    auto report1 = ExecutionReport(id, timestamp1);
    auto timestamp2 = timestamp1 + seconds(1);
    auto report2 = make_update(report1, OrderStatus::NEW, timestamp2);
    auto timestamp3 = timestamp2 + seconds(1);
    auto report3 =
      make_update(report2, OrderStatus::PARTIALLY_FILLED, timestamp3);
    report3.m_last_quantity = 50;
    auto record = OrderRecord(info, {report1, report2, report3});
    fixture.on_request<LoadOrderByIdService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == id);
        request.SetResult(make_optional(SequencedValue(
          IndexedValue(record, DirectoryEntry::MakeAccount(123, "user")),
          Beam::Queries::Sequence(555))));
      });
    auto received_order = REQUIRE_NO_THROW(fixture.m_client->load_order(id));
    REQUIRE(received_order);
    REQUIRE(received_order->get_info() == info);
    auto execution_reports = *received_order->get_publisher().GetSnapshot();
    REQUIRE(execution_reports == record.m_execution_reports);
    auto duplicate_order = REQUIRE_NO_THROW(fixture.m_client->load_order(id));
    REQUIRE(duplicate_order == received_order);
  }

  TEST_CASE("submit") {
    auto fixture = Fixture();
    auto fields = make_order_fields();
    auto id = 432;
    auto timestamp = time_from_string("2024-05-21 00:00:10.000");
    auto info = OrderInfo(fields, id, timestamp);
    fixture.on_request<NewOrderSingleService>(
      [&] (auto& request, const auto& received_fields) {
        REQUIRE(received_fields == fields);
        request.SetResult(SequencedValue(
          IndexedValue(info, DirectoryEntry::MakeAccount(123, "user")),
          Beam::Queries::Sequence(556)));
      });
    fixture.on_request<QueryOrderSubmissionsService>(
      [&] (auto& request, const auto& query) {
        auto result = OrderSubmissionQueryResult();
        result.m_queryId = 123;
        request.SetResult(result);
      });
    auto received_order = REQUIRE_NO_THROW(fixture.m_client->submit(fields));
    REQUIRE(received_order);
    REQUIRE(received_order->get_info() == info);
    auto execution_reports = *received_order->get_publisher().GetSnapshot();
    REQUIRE(execution_reports.empty());
  }

  TEST_CASE("cancel") {
    auto fixture = Fixture();
    auto id = 433;
    auto timestamp = time_from_string("2024-05-21 00:00:10.000");
    auto fields = make_order_fields();
    auto info = OrderInfo(fields, id, timestamp);
    auto order = PrimitiveOrder(info);
    auto received_cancel = false;
    auto completion_token = Async<void>();
    fixture.on_message<CancelOrderMessage>(
      [&] (auto& client, auto received_id) {
        REQUIRE(received_id == id);
        received_cancel = true;
        completion_token.GetEval().SetResult();
      });
    REQUIRE_NOTHROW(fixture.m_client->cancel(order));
    completion_token.Get();
  }
}
