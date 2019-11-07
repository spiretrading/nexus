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
  auto make(const std::string& symbol, const ptime& timestamp) {
    return OrderImbalance(Security(symbol, 0), Side::BID, 100,
      Money(1 * Money::ONE), timestamp);
  }

  auto make(const std::string& symbol, const MarketCode& market, Side side,
      Quantity size, double ref_price, const ptime& timestamp) {
    return OrderImbalance(Security(symbol, market, 0), side, size,
      Money(Quantity(ref_price)), timestamp);
  }

  auto market_db = GetDefaultMarketDatabase();

  const auto a = make("A", market_db.FromDisplayName("TSX").m_code, Side::BID,
    100, 1.0, from_time_t(100));
  const auto b = make("B", market_db.FromDisplayName("TSX").m_code, Side::BID,
    1000, 10.0, from_time_t(200));
  const auto c = make("C", market_db.FromDisplayName("TSX").m_code, Side::ASK,
    10000, 100.0, from_time_t(300));
  const auto d = make("D", market_db.FromDisplayName("NYSE").m_code, Side::ASK,
    100000, 1000.0, from_time_t(400));
  const auto e = make("E", market_db.FromDisplayName("NYSE").m_code, Side::ASK,
    1000000, 10000.0, from_time_t(500));

  const auto market_database = GetDefaultMarketDatabase();
}

auto make_local_model() {
    auto model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    model->insert(a);
    model->insert(b);
    model->insert(c);
    model->insert(d);
    model->insert(e);
    return model;
}

TEST_CASE("test_unfiltered_subscribing",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = FilteredOrderImbalanceIndicatorModel(make_local_model(), {});
    auto [connection1, promise1] = model.subscribe(from_time_t(100),
      from_time_t(300), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({a, b, c}));
    auto [connection2, promise2] = model.subscribe(from_time_t(200),
      from_time_t(400), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({b, c, d}));
    auto [connection3, promise3] = model.subscribe(from_time_t(300),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3 == std::vector<OrderImbalance>({c, d, e}));
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
    REQUIRE(data1 == std::vector<OrderImbalance>({a}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_security_list_filter({"A", "C", "E"})});
    auto [connection2, promise2] = model2.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({a, c, e}));
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
    REQUIRE(data1 == std::vector<OrderImbalance>({a}));
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
    REQUIRE(data3 == std::vector<OrderImbalance>({a, b, c, d, e}));
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
    REQUIRE(data1 == std::vector<OrderImbalance>({a, b, c}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_list_filter({"TSX", "NYSE"})});
    auto [connection2, promise2] = model2.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({a, b, c, d, e}));
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
    REQUIRE(data1 == std::vector<OrderImbalance>({a, b, c}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_filter({"N"})});
    auto [connection2, promise2] = model2.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({d, e}));
    auto model3 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_market_filter({})});
    auto [connection3, promise3] = model3.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3 == std::vector<OrderImbalance>({a, b, c, d, e}));
  }, "test_market_filter");
}
