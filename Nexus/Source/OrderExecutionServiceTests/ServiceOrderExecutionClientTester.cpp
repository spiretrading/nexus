#include <Beam/ServicesTests/ServiceClientFixture.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/ServiceOrderExecutionClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

namespace {
  auto make_order_fields() {
    return make_limit_order_fields(
      Security("TST", TSX), Side::BID, "TSX", Quantity(100), Money::ONE);
  }

  struct Fixture : ServiceClientFixture {
    using TestServiceOrderExecutionClient =
      ServiceOrderExecutionClient<TestServiceProtocolClientBuilder>;
    std::unique_ptr<TestServiceOrderExecutionClient> m_client;

    Fixture() {
      Nexus::register_query_types(out(m_server.get_slots().get_registry()));
      register_order_execution_services(out(m_server.get_slots()));
      register_order_execution_messages(out(m_server.get_slots()));
      m_client = make_client<TestServiceOrderExecutionClient>();
    }
  };
}

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
        request.set(make_optional(SequencedValue(IndexedValue(record,
          DirectoryEntry::make_account(123, "user")), Beam::Sequence(555))));
      });
    auto received_order = REQUIRE_NO_THROW(fixture.m_client->load_order(id));
    REQUIRE(received_order);
    REQUIRE(received_order->get_info() == info);
    auto execution_reports = *received_order->get_publisher().get_snapshot();
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
        request.set(SequencedValue(IndexedValue(info,
          DirectoryEntry::make_account(123, "user")), Beam::Sequence(556)));
      });
    fixture.on_request<QueryOrderSubmissionsService>(
      [&] (auto& request, const auto& query) {
        auto result = OrderSubmissionQueryResult();
        result.m_id = 123;
        request.set(result);
      });
    auto received_order = REQUIRE_NO_THROW(fixture.m_client->submit(fields));
    REQUIRE(received_order);
    REQUIRE(received_order->get_info() == info);
    auto execution_reports = *received_order->get_publisher().get_snapshot();
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
        completion_token.get_eval().set();
      });
    REQUIRE_NOTHROW(fixture.m_client->cancel(order));
    completion_token.get();
  }
}
