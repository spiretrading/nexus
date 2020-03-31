#include <catch2/catch.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Spire/OrderImbalanceIndicator/FilteredOrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/SpireTester/TestOrderImbalanceIndicatorModel.hpp"

using namespace boost::posix_time;
using Filter = Spire::FilteredOrderImbalanceIndicatorModel::Filter;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_imbalance(const std::string& symbol, const ptime& timestamp) {
    return OrderImbalance(Security(symbol, 0), Side::BID, 100, Money::ONE,
      timestamp);
  }

  auto make_imbalance(const Security& security, const ptime& timestamp) {
    return OrderImbalance(security, Side::BID, 100, Money::ONE, timestamp);
  }

  auto make_imbalance(const Security& security, const MarketCode& market,
      Side side, Quantity size, Money ref_price, const ptime& timestamp) {
    return OrderImbalance(security, side, size, ref_price, timestamp);
  }

  auto make_imbalance(const std::string& symbol, const MarketCode& market,
      Side side, Quantity size, Money ref_price, const ptime& timestamp) {
    return OrderImbalance(Security(symbol, market, 0), side, size, ref_price,
      timestamp);
  }

  auto market_db = GetDefaultMarketDatabase();

  const auto A = make_imbalance("A", market_db.FromDisplayName("TSX").m_code,
    Side::BID, 100, Money::ONE, from_time_t(100));
  const auto B = make_imbalance("B", market_db.FromDisplayName("TSX").m_code,
    Side::BID, 1000, 10 * Money::ONE, from_time_t(200));
  const auto C = make_imbalance("C", market_db.FromDisplayName("TSX").m_code,
    Side::ASK, 10000, 100 * Money::ONE, from_time_t(300));
  const auto D = make_imbalance("D", market_db.FromDisplayName("NYSE").m_code,
    Side::ASK, 100000, 1000 * Money::ONE, from_time_t(400));
  const auto E = make_imbalance("E", market_db.FromDisplayName("NYSE").m_code,
    Side::ASK, 1000000, 10000 * Money::ONE, from_time_t(500));

  const auto market_database = GetDefaultMarketDatabase();

  auto make_local_model() {
    auto model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    model->insert(A);
    model->insert(B);
    model->insert(C);
    model->insert(D);
    model->insert(E);
    return model;
  }
}

TEST_CASE("test_unfiltered_loading",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = FilteredOrderImbalanceIndicatorModel(make_local_model(), {});
    auto promise1 = model.load(TimeInterval::closed(
      from_time_t(100), from_time_t(300)));
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A, B, C}));
    auto promise2 = model.load(TimeInterval::closed(
      from_time_t(200), from_time_t(400)));
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({B, C, D}));
    auto promise3 = model.load(TimeInterval::closed(from_time_t(300),
      from_time_t(500)));
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3 == std::vector<OrderImbalance>({C, D, E}));
  }, "test_unfiltered_loading");
}

TEST_CASE("test_security_list_filter",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_security_list_filter({A.m_security})});
    auto promise1 = model1.load(TimeInterval::closed(
      from_time_t(0), from_time_t(500)));
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_security_list_filter({A.m_security, C.m_security, E.m_security})});
    auto promise2 = model2.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({A, C, E}));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_security_list_filter({})});
    auto promise3 = model3.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3.empty());
  }, "test_security_list_filter");
}

TEST_CASE("test_security_list_filter_with_duplicate_symbols",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto country_db = GetDefaultCountryDatabase();
    auto first_imbalance = OrderImbalance(Security("A",
      market_db.FromDisplayName("TSX").m_code,
      ParseCountryCode("CA", country_db)), Side::BID, 100,
      Money(Quantity(10)), from_time_t(100));
    auto second_imbalance = OrderImbalance(Security("A",
      market_db.FromDisplayName("NYSE").m_code,
      ParseCountryCode("US", country_db)), Side::BID, 100,
      Money(Quantity(10)), from_time_t(100));
    auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    local_model->insert(first_imbalance);
    local_model->insert(second_imbalance);
    auto filtered_model = FilteredOrderImbalanceIndicatorModel(local_model,
      {make_security_list_filter({first_imbalance.m_security})});
    auto promise = filtered_model.load(TimeInterval::closed(
      from_time_t(0), from_time_t(500)));
    auto data = wait(std::move(promise));
    REQUIRE(data == std::vector<OrderImbalance>({first_imbalance}));
  }, "test_security_list_filter_with_duplicate_symbols");
}

TEST_CASE("test_security_filter", "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_symbol_filter("A")});
    auto promise1 = model1.load(TimeInterval::closed(
      from_time_t(0), from_time_t(500)));
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_symbol_filter("AZ")});
    auto promise2 = model2.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2.empty());
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_symbol_filter({""})});
    auto promise3 = model3.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3 == std::vector<OrderImbalance>({A, B, C, D, E}));
  }, "test_security_filter");
}

TEST_CASE("test_market_list_filter",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_list_filter({"TSX"}, GetDefaultMarketDatabase())});
    auto promise1 = model1.load(TimeInterval::closed(
      from_time_t(0), from_time_t(500)));
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A, B, C}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_list_filter({"TSX", "NYSE"}, GetDefaultMarketDatabase())});
    auto promise2 = model2.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({A, B, C, D, E}));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_list_filter({}, GetDefaultMarketDatabase())});
    auto promise3 = model3.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3.empty());
  }, "test_market_list_filter");
}

TEST_CASE("test_market_filter", "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_filter("T", GetDefaultMarketDatabase())});
    auto promise1 = model1.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A, B, C}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_filter("N", GetDefaultMarketDatabase())});
    auto promise2 = model2.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({D, E}));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_filter("", GetDefaultMarketDatabase())});
    auto promise3 = model3.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3 == std::vector<OrderImbalance>({A, B, C, D, E}));
  }, "test_market_filter");
}

TEST_CASE("test_side_filter", "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_side_filter(Side::BID)});
    auto promise1 = model1.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A, B}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_side_filter(Side::ASK)});
    auto promise2 = model2.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({C, D, E}));
  }, "test_side_filter");
}

TEST_CASE("test_size_filter", "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_size_filter({0}, {500})});
    auto promise1 = model1.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_size_filter({0}, {10000})});
    auto promise2 = model2.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({A, B, C}));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_size_filter({100000000}, {100000000})});
    auto promise3 = model3.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3.empty());
  }, "test_size_filter");
}

TEST_CASE("test_reference_price_filter",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_reference_price_filter(Money(Money::ZERO), Money(Money::ONE))});
    auto promise1 = model1.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_reference_price_filter(Money(Money::ZERO),
      Money(100 * Money::ONE))});
    auto promise2 = model2.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({A, B, C}));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_reference_price_filter(Money(100000000 * Money::ONE),
      Money(100000000 * Money::ONE))});
    auto promise3 = model3.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3.empty());
  }, "test_reference_price_filter");
}

TEST_CASE("test_notional_value_filter",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_notional_value_filter(Money(Money::ONE),
      Money(100 * Money::ONE))});
    auto promise1 = model1.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_notional_value_filter(Money(Money::ONE),
      Money(1000000 * Money::ONE))});
    auto promise2 = model2.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({A, B, C}));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_notional_value_filter(Money(Money::ONE),
      Money(10 * Money::ONE))});
    auto promise3 = model3.load(TimeInterval::closed(from_time_t(0),
      from_time_t(500)));
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3.empty());
  }, "test_notional_value_filter");
}

TEST_CASE("test_unfiltered_signals",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    auto model = FilteredOrderImbalanceIndicatorModel(local_model, {});
    auto slot_data = OrderImbalance();
    model.subscribe([&] (const auto& imbalance) { slot_data = imbalance; });
    local_model->publish(A);
    REQUIRE(slot_data == A);
    local_model->publish(B);
    REQUIRE(slot_data == B);
  }, "test_unfiltered_signals");
}

TEST_CASE("test_filtered_signals",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    auto model = FilteredOrderImbalanceIndicatorModel(local_model,
      {make_market_list_filter({"TSX"}, GetDefaultMarketDatabase())});
    auto signal_data = OrderImbalance();
    auto [connection, promise] = model.subscribe([&] (auto& i) {
      signal_data = i; });
    wait(std::move(promise));
    REQUIRE(signal_data == OrderImbalance());
    local_model->publish(A);
    REQUIRE(signal_data == A);
    local_model->publish(B);
    REQUIRE(signal_data == B);
    local_model->publish(C);
    REQUIRE(signal_data == C);
    local_model->publish(D);
    REQUIRE(signal_data == C);
    local_model->publish(E);
    REQUIRE(signal_data == C);
  }, "test_filtered_signals");
}

TEST_CASE("test_filtered_loads_don't_crash_on_model_destruction",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
    auto filtered_model = std::make_unique<FilteredOrderImbalanceIndicatorModel>(
      test_model, std::vector<FilteredOrderImbalanceIndicatorModel::Filter>());
    auto promise = filtered_model->load(TimeInterval::closed(from_time_t(0),
      from_time_t(1000)));
    filtered_model.reset();
    auto test_load = wait(test_model->pop_load());
    test_load->set_result({A, B, C});
    wait(std::move(promise));
  }, "test_filtered_loads_don't_crash_on_model_destruction");
}

TEST_CASE("test_filtered_subscribes_don't_crash_on_model_destruction",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    auto filtered_model =
      std::make_unique<FilteredOrderImbalanceIndicatorModel>(local_model,
      std::vector<FilteredOrderImbalanceIndicatorModel::Filter>());
    auto signal_data = OrderImbalance();
    auto promise = filtered_model->subscribe([&] (const auto& imbalance) {
      signal_data = imbalance; });
    filtered_model.reset();
    local_model->publish(A);
    wait(std::move(promise.m_snapshot));
  }, "test_filtered_subscribes_don't_crash_on_model_destruction");
}

TEST_CASE("test_unfiltered_single_security_loading",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto local_model = make_local_model();
    const auto S = Security("TEST", 0);
    const auto F = make_imbalance(S, from_time_t(100));
    const auto G = make_imbalance(S, from_time_t(200));
    const auto H = make_imbalance(S, from_time_t(300));
    local_model->insert(F);
    local_model->insert(G);
    local_model->insert(H);
    auto filtered_model = FilteredOrderImbalanceIndicatorModel(local_model,
      {});
    auto promise = filtered_model.load(S, TimeInterval::closed(
      from_time_t(100), from_time_t(500)));
    auto data = wait(std::move(promise));
    REQUIRE(data == std::vector<OrderImbalance>({F, G, H}));
  }, "test_unfiltered_single_security_loading");
}

TEST_CASE("test_filtered_single_security_loading",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto local_model = make_local_model();
    const auto S = Security("TEST", 0);
    const auto F = make_imbalance(S, "", Side::ASK, 100, 10 * Money::ONE,
      from_time_t(100));
    const auto G = make_imbalance(S, "", Side::ASK, 200, 10 * Money::ONE,
      from_time_t(100));
    const auto H = make_imbalance(S, "", Side::ASK, 300, 10 * Money::ONE,
      from_time_t(100));
    local_model->insert(F);
    local_model->insert(G);
    local_model->insert(H);
    auto filtered_model = FilteredOrderImbalanceIndicatorModel(local_model,
      {make_size_filter(150, 250)});
    auto promise = filtered_model.load(S, TimeInterval::closed(
      from_time_t(100), from_time_t(500)));
    auto data = wait(std::move(promise));
    REQUIRE(data == std::vector<OrderImbalance>({G}));
  }, "test_filtered_single_security_loading");
}
