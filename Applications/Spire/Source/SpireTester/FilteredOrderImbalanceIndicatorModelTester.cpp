#include <catch.hpp>
#include "Spire/OrderImbalanceIndicator/FilteredOrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using boost::posix_time::ptime;
using boost::posix_time::from_time_t;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_imbalance(const std::string& symbol, const ptime& timestamp) {
    return OrderImbalance(Security(symbol, 0), Side::BID, 100,
      Money(1 * Money::ONE), timestamp);
  }

  auto make_imbalance(const std::string& symbol, const MarketCode& market,
      Side side, Quantity size, double ref_price, const ptime& timestamp) {
    return OrderImbalance(Security(symbol, market, 0), side, size,
      Money(Quantity(ref_price)), timestamp);
  }

  auto market_db = GetDefaultMarketDatabase();

  const auto A = make_imbalance("A", market_db.FromDisplayName("TSX").m_code,
    Side::BID, 100, 1.0, from_time_t(100));
  const auto B = make_imbalance("B", market_db.FromDisplayName("TSX").m_code,
    Side::BID, 1000, 10.0, from_time_t(200));
  const auto C = make_imbalance("C", market_db.FromDisplayName("TSX").m_code,
    Side::ASK, 10000, 100.0, from_time_t(300));
  const auto D = make_imbalance("D", market_db.FromDisplayName("NYSE").m_code,
    Side::ASK, 100000, 1000.0, from_time_t(400));
  const auto E = make_imbalance("E", market_db.FromDisplayName("NYSE").m_code,
    Side::ASK, 1000000, 10000.0, from_time_t(500));

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

TEST_CASE("test_unfiltered_subscribing",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = FilteredOrderImbalanceIndicatorModel(make_local_model(), {});
    auto [connection1, promise1] = model.subscribe(from_time_t(100),
      from_time_t(300), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    auto expected1 = std::vector<OrderImbalance>({A, B, C});
    REQUIRE(std::is_permutation(data1.begin(), data1.end(), expected1.begin(),
      expected1.end()));
    auto [connection2, promise2] = model.subscribe(from_time_t(200),
      from_time_t(400), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({B, C, D});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
    auto [connection3, promise3] = model.subscribe(from_time_t(300),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    auto expected3 = std::vector<OrderImbalance>({C, D, E});
    REQUIRE(std::is_permutation(data3.begin(), data3.end(), expected3.begin(),
      expected3.end()));
  }, "test_unfiltered_subscribing");
}

TEST_CASE("test_security_list_filter",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_security_list_filter({"A"})});
    auto [connection1, promise1] = model1.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_security_list_filter({"A", "C", "E"})});
    auto [connection2, promise2] = model2.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({A, C, E});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_security_list_filter({})});
    auto [connection3, promise3] = model3.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3.empty());
  }, "test_security_list_filter");
}

TEST_CASE("test_security_filter", "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_security_filter({"A"})});
    auto [connection1, promise1] = model1.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_security_filter({"AZ"})});
    auto [connection2, promise2] = model2.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2.empty());
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_security_filter({""})});
    auto [connection3, promise3] = model3.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    auto expected3 = std::vector<OrderImbalance>({A, B, C, D, E});
    REQUIRE(std::is_permutation(data3.begin(), data3.end(), expected3.begin(),
      expected3.end()));
  }, "test_security_filter");
}

TEST_CASE("test_market_list_filter",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_list_filter({"TSX"})});
    auto [connection1, promise1] = model1.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    auto expected1 = std::vector<OrderImbalance>({A, B, C});
    REQUIRE(std::is_permutation(data1.begin(), data1.end(), expected1.begin(),
      expected1.end()));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_list_filter({"TSX", "NYSE"})});
    auto [connection2, promise2] = model2.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({A, B, C, D, E});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_list_filter({})});
    auto [connection3, promise3] = model3.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3.empty());
  }, "test_market_list_filter");
}

TEST_CASE("test_market_filter", "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_filter({"T"})});
    auto [connection1, promise1] = model1.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    auto expected1 = std::vector<OrderImbalance>({A, B, C});
    REQUIRE(std::is_permutation(data1.begin(), data1.end(), expected1.begin(),
      expected1.end()));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_filter({"N"})});
    auto [connection2, promise2] = model2.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({D, E});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_filter({})});
    auto [connection3, promise3] = model3.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    auto expected3 = std::vector<OrderImbalance>({A, B, C, D, E});
    REQUIRE(std::is_permutation(data3.begin(), data3.end(), expected3.begin(),
      expected3.end()));
  }, "test_market_filter");
}

TEST_CASE("test_side_filter", "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_side_filter(Side::BID)});
    auto [connection1, promise1] = model1.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    auto expected1 = std::vector<OrderImbalance>({A, B});
    REQUIRE(std::is_permutation(data1.begin(), data1.end(), expected1.begin(),
      expected1.end()));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_side_filter(Side::ASK)});
    auto [connection2, promise2] = model2.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({C, D, E});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
  }, "test_side_filter");
}

TEST_CASE("test_size_filter", "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_size_filter({0}, {500})});
    auto [connection1, promise1] = model1.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_size_filter({0}, {10000})});
    auto [connection2, promise2] = model2.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({A, B, C});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_size_filter({100000000}, {100000000})});
    auto [connection3, promise3] = model3.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3.empty());
  }, "test_size_filter");
}

TEST_CASE("test_reference_price_filter",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_reference_price_filter(Money(Money::ZERO), Money(Money::ONE))});
    auto [connection1, promise1] = model1.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_reference_price_filter(Money(Money::ZERO),
      Money(100 * Money::ONE))});
    auto [connection2, promise2] = model2.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({A, B, C});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_reference_price_filter(Money(100000000 * Money::ONE),
      Money(100000000 * Money::ONE))});
    auto [connection3, promise3] = model3.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3.empty());
  }, "test_reference_price_filter");
}

TEST_CASE("test_notional_value_filter",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_notional_value_filter(Money(Money::ONE), Money(100 * Money::ONE))});
    auto [connection1, promise1] = model1.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({A}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_notional_value_filter(Money(Money::ONE),
      Money(1000000 * Money::ONE))});
    auto [connection2, promise2] = model2.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({A, B, C});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_notional_value_filter(Money(Money::ONE), Money(10 * Money::ONE))});
    auto [connection3, promise3] = model3.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3.empty());
  }, "test_notional_value_filter");
}

TEST_CASE("test_filtered_signals",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    auto model = FilteredOrderImbalanceIndicatorModel(local_model,
      {make_market_list_filter({"TSX"})});
    auto signal_data1 = OrderImbalance();
    auto signal_data2 = OrderImbalance();
    auto signal_data3 = OrderImbalance();
    auto signal_data4 = OrderImbalance();
    auto [connection1, promise1] = model.subscribe(from_time_t(100),
      from_time_t(500), [&] (auto& i) { signal_data1 = i; });
    wait(std::move(promise1));
    auto [connection2, promise2] = model.subscribe(from_time_t(200),
      from_time_t(200), [&] (auto& i) { signal_data2 = i; });
    wait(std::move(promise2));
    auto [connection3, promise3] = model.subscribe(from_time_t(300),
      from_time_t(400), [&] (auto& i) { signal_data3 = i; });
    wait(std::move(promise3));
    auto [connection4, promise4] = model.subscribe(from_time_t(900),
      from_time_t(1000), [&] (auto& i) { signal_data4 = i; });
    wait(std::move(promise4));
    local_model->insert(A);
    REQUIRE(signal_data1 == A);
    REQUIRE(signal_data2 == OrderImbalance());
    REQUIRE(signal_data3 == OrderImbalance());
    REQUIRE(signal_data4 == OrderImbalance());
    local_model->insert(B);
    REQUIRE(signal_data1 == B);
    REQUIRE(signal_data2 == B);
    REQUIRE(signal_data3 == OrderImbalance());
    REQUIRE(signal_data4 == OrderImbalance());
    local_model->insert(C);
    REQUIRE(signal_data1 == C);
    REQUIRE(signal_data2 == B);
    REQUIRE(signal_data3 == C);
    REQUIRE(signal_data4 == OrderImbalance());
    local_model->insert(D);
    REQUIRE(signal_data1 == C);
    REQUIRE(signal_data2 == B);
    REQUIRE(signal_data3 == C);
    REQUIRE(signal_data4 == OrderImbalance());
  }, "test_filtered_signals");
}
