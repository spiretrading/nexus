#include <future>
#include <doctest/doctest.h>
#include <boost/functional/factory.hpp>
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/RiskService/RiskTransitionModel.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

namespace {
  auto ACCOUNT = DirectoryEntry::make_account(153, "simba");
  auto S32 = parse_ticker("S32.ASX");
  auto XIU = parse_ticker("XIU.TSX");
}

TEST_SUITE("RiskTransitionModel") {
  TEST_CASE("cancel_opening_orders") {
    auto operations = std::make_shared<TestOrderExecutionClient::Queue>();
    auto client = TestOrderExecutionClient(operations);
    auto model = RiskTransitionModel(
      ACCOUNT, {}, RiskState::Type::ACTIVE, &client, DEFAULT_DESTINATIONS);
    auto bid_order = std::make_shared<PrimitiveOrder>(
      OrderInfo(make_limit_order_fields(S32, AUD, Side::BID, 100, Money::ONE),
        112, time_from_string("2020-11-17 12:22:06")));
    model.add(bid_order);
    auto bid_report = ExecutionReport();
    bid_order->with([&] (auto state, const auto& reports) {
      bid_report = reports.front();
    });
    model.update(bid_report);
    bid_report =
      make_update(bid_report, OrderStatus::NEW, bid_report.m_timestamp);
    model.update(bid_report);
    auto ask_order =
      std::make_shared<PrimitiveOrder>(OrderInfo(make_limit_order_fields(
        S32, AUD, Side::ASK, 100, Money::ONE + Money::CENT),
      113, time_from_string("2020-11-17 12:22:06")));
    model.add(ask_order);
    auto ask_report = ExecutionReport();
    ask_order->with([&] (auto state, const auto& reports) {
      ask_report = reports.front();
    });
    model.update(ask_report);
    ask_report =
      make_update(ask_report, OrderStatus::NEW, ask_report.m_timestamp);
    model.update(ask_report);
    model.update(RiskState::Type::CLOSE_ORDERS);
    auto cancel_ids = std::vector<OrderId>();
    auto operation = operations->pop();
    auto cancel_operation =
      std::get_if<TestOrderExecutionClient::CancelOperation>(&*operation);
    REQUIRE(cancel_operation);
    cancel_ids.push_back(cancel_operation->m_id);
    operation = operations->pop();
    cancel_operation =
      std::get_if<TestOrderExecutionClient::CancelOperation>(&*operation);
    REQUIRE(cancel_operation);
    cancel_ids.push_back(cancel_operation->m_id);
    auto expected_cancel_ids = std::vector<OrderId>{112, 113};
    REQUIRE(std::is_permutation(cancel_ids.begin(), cancel_ids.end(),
      expected_cancel_ids.begin(), expected_cancel_ids.end()));
  }

  TEST_CASE("flatten_disabled") {
    auto operations = std::make_shared<TestOrderExecutionClient::Queue>();
    auto client = TestOrderExecutionClient(operations);
    auto model = RiskTransitionModel(
      ACCOUNT, {}, RiskState::Type::ACTIVE, &client, DEFAULT_DESTINATIONS);
    auto bid_order = std::make_shared<PrimitiveOrder>(
      OrderInfo(make_limit_order_fields(S32, AUD, Side::BID, 100, Money::ONE),
        112, time_from_string("2020-11-17 12:22:06")));
    model.add(bid_order);
    auto bid_report = ExecutionReport();
    bid_order->with([&] (auto state, const auto& reports) {
      bid_report = reports.front();
    });
    model.update(bid_report);
    bid_report =
      make_update(bid_report, OrderStatus::NEW, bid_report.m_timestamp);
    model.update(bid_report);
    bid_report =
      make_update(bid_report, OrderStatus::FILLED, bid_report.m_timestamp);
    bid_report.m_last_price = Money::ONE;
    bid_report.m_last_quantity = 100;
    model.update(bid_report);
    auto bid_order2 = std::make_shared<PrimitiveOrder>(
      OrderInfo(make_limit_order_fields(S32, AUD, Side::BID, 100, Money::ONE),
        127, time_from_string("2020-11-17 12:22:06")));
    model.add(bid_order2);
    auto bid_report2 = ExecutionReport();
    bid_order2->with([&] (auto state, const auto& reports) {
      bid_report2 = reports.front();
    });
    model.update(bid_report2);
    bid_report2 =
      make_update(bid_report2, OrderStatus::NEW, bid_report2.m_timestamp);
    model.update(bid_report2);
    auto ask_order = std::make_shared<PrimitiveOrder>(
      OrderInfo(make_limit_order_fields(S32, AUD, Side::ASK, 100,
        Money::ONE + Money::CENT),
      143, time_from_string("2020-11-17 12:22:06")));
    model.add(ask_order);
    auto ask_report = ExecutionReport();
    ask_order->with([&] (auto state, const auto& reports) {
      ask_report = reports.front();
    });
    model.update(ask_report);
    ask_report =
      make_update(ask_report, OrderStatus::NEW, ask_report.m_timestamp);
    model.update(ask_report);
    model.update(RiskState::Type::CLOSE_ORDERS);
    auto operation = operations->pop();
    auto cancel_operation =
      std::get_if<TestOrderExecutionClient::CancelOperation>(&*operation);
    REQUIRE(cancel_operation);
    REQUIRE(cancel_operation->m_id == 127);
    bid_report2 =
      make_update(bid_report2, OrderStatus::CANCELED, bid_report2.m_timestamp);
    bid_report2.m_id = 127;
    model.update(bid_report2);
    model.update(RiskState::Type::DISABLED);
    operation = operations->pop();
    cancel_operation =
      std::get_if<TestOrderExecutionClient::CancelOperation>(&*operation);
    REQUIRE(cancel_operation);
    REQUIRE(cancel_operation->m_id == 143);
    ask_report =
      make_update(ask_report, OrderStatus::CANCELED, ask_report.m_timestamp);
    auto submit_async = std::async(std::launch::async, [&] {
      model.update(ask_report);
    });
    operation = operations->pop();
    auto submit_operation =
      std::get_if<TestOrderExecutionClient::SubmitOperation>(&*operation);
    REQUIRE(submit_operation);
    REQUIRE(submit_operation->m_fields.m_ticker == S32);
    REQUIRE(submit_operation->m_fields.m_side == Side::ASK);
    REQUIRE(submit_operation->m_fields.m_quantity == 100);
    REQUIRE(submit_operation->m_fields.m_type == OrderType::MARKET);
    REQUIRE(submit_operation->m_fields.m_destination == "ASXT");
    submit_operation->m_result.set(std::make_shared<PrimitiveOrder>(
      OrderInfo(submit_operation->m_fields, 100,
        time_from_string("2020-11-17 12:22:06"))));
    submit_async.get();
  }

  TEST_CASE("initial_inventory") {
    auto inventory = std::vector<Inventory>();
    inventory.push_back(Inventory(Position(XIU, CAD, -300, 300 * Money::ONE),
      Money::ONE, Money::ZERO, 300, 1));
    auto operations = std::make_shared<TestOrderExecutionClient::Queue>();
    auto client = TestOrderExecutionClient(operations);
    auto model = RiskTransitionModel(ACCOUNT, inventory,
      RiskState::Type::ACTIVE, &client, DEFAULT_DESTINATIONS);
    auto bid_order = std::make_shared<PrimitiveOrder>(
      OrderInfo(make_limit_order_fields(XIU, CAD, Side::BID, 300, Money::ONE),
        113, time_from_string("2020-11-17 12:22:06")));
    model.add(bid_order);
    auto bid_report = ExecutionReport();
    bid_order->with([&] (auto state, const auto& reports) {
      bid_report = reports.front();
    });
    model.update(bid_report);
    bid_report =
      make_update(bid_report, OrderStatus::NEW, bid_report.m_timestamp);
    model.update(bid_report);
    auto bid_order2 = std::make_shared<PrimitiveOrder>(OrderInfo(
      make_limit_order_fields(XIU, CAD, Side::BID, 100, 2 * Money::ONE),
      114, time_from_string("2020-11-17 12:22:06")));
    model.add(bid_order2);
    auto bid_report2 = ExecutionReport();
    bid_order2->with([&] (auto state, const auto& reports) {
      bid_report2 = reports.front();
    });
    model.update(bid_report2);
    bid_report2 =
      make_update(bid_report2, OrderStatus::NEW, bid_report2.m_timestamp);
    model.update(bid_report2);
    model.update(RiskState::Type::CLOSE_ORDERS);
    auto operation = operations->pop();
    auto cancel_operation =
      std::get_if<TestOrderExecutionClient::CancelOperation>(&*operation);
    REQUIRE(cancel_operation);
    REQUIRE(cancel_operation->m_id == 114);
    bid_report2 =
      make_update(bid_report2, OrderStatus::CANCELED, bid_report2.m_timestamp);
    model.update(bid_report2);
    auto sync_order = std::make_shared<PrimitiveOrder>(OrderInfo(
      make_limit_order_fields(
        XIU, CAD, Side::ASK, 100, Money::ONE + Money::CENT),
      1000, time_from_string("2020-11-17 12:22:06")));
    client.cancel(*sync_order);
    operation = operations->pop();
    cancel_operation =
      std::get_if<TestOrderExecutionClient::CancelOperation>(&*operation);
    REQUIRE(cancel_operation);
    REQUIRE(cancel_operation->m_id == 1000);
    model.update(RiskState::Type::DISABLED);
    operation = operations->pop();
    cancel_operation =
      std::get_if<TestOrderExecutionClient::CancelOperation>(&*operation);
    REQUIRE(cancel_operation);
    REQUIRE(cancel_operation->m_id == 113);
    bid_report =
      make_update(bid_report, OrderStatus::CANCELED, bid_report.m_timestamp);
    auto submit_async = std::async(std::launch::async, [&] {
      model.update(bid_report);
    });
    operation = operations->pop();
    auto submit_operation =
      std::get_if<TestOrderExecutionClient::SubmitOperation>(&*operation);
    REQUIRE(submit_operation);
    REQUIRE(submit_operation->m_fields.m_ticker == XIU);
    REQUIRE(submit_operation->m_fields.m_side == Side::BID);
    REQUIRE(submit_operation->m_fields.m_quantity == 300);
    REQUIRE(submit_operation->m_fields.m_type == OrderType::MARKET);
    REQUIRE(submit_operation->m_fields.m_destination == "TSX");
    submit_operation->m_result.set(std::make_shared<PrimitiveOrder>(
      OrderInfo(submit_operation->m_fields, 100,
        time_from_string("2020-11-17 12:22:06"))));
    submit_async.get();
  }
}
