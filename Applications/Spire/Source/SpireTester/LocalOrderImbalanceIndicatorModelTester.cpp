#include <catch.hpp>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using boost::posix_time::ptime;
using boost::posix_time::from_time_t;
using namespace Nexus;
using namespace Spire;

auto make(const std::string& symbol, const ptime& timestamp) {
  return OrderImbalance(Security(symbol, 0), Side::BID, 100,
    Money(1 * Money::ONE), timestamp);
}

TEST_CASE("test_basic_inserting_subscribing",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto a = make("A", from_time_t(100));
    auto b = make("B", from_time_t(200));
    auto c = make("C", from_time_t(300));
    model.insert(a);
    model.insert(b);
    model.insert(c);
    auto [connection, promise] = model.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data = wait(std::move(promise));
    REQUIRE(data == std::vector<OrderImbalance>({a, b, c}));
  }, "test_basic_inserting_subscribing");
}

TEST_CASE("test_subscribing_with_specific_range",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto a = make("A", from_time_t(100));
    auto b = make("B", from_time_t(200));
    auto c = make("C", from_time_t(300));
    auto d = make("D", from_time_t(400));
    auto e = make("E", from_time_t(500));
    model.insert(a);
    model.insert(b);
    model.insert(c);
    model.insert(d);
    model.insert(e);
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
  }, "test_subscribing_with_specific_range");
}

TEST_CASE("test_order_imbalance_signals",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto a = make("A", from_time_t(100));
    auto b = make("B", from_time_t(200));
    auto c = make("C", from_time_t(300));
    auto d = make("D", from_time_t(400));
    auto e = make("E", from_time_t(500));
    auto signal_data1 = OrderImbalance();
    auto signal_data2 = OrderImbalance();
    auto signal_data3 = OrderImbalance();
    auto [connection1, promise1] = model.subscribe(from_time_t(100),
      from_time_t(300), [&] (auto& i) { signal_data1 = i; });
    wait(std::move(promise1));
    auto [connection2, promise2] = model.subscribe(from_time_t(200),
      from_time_t(400), [&] (auto& i) { signal_data2 = i; });
    wait(std::move(promise2));
    auto [connection3, promise3] = model.subscribe(from_time_t(300),
      from_time_t(500), [&] (auto& i) { signal_data3 = i; });
    wait(std::move(promise3));
    model.insert(a);
    REQUIRE(signal_data1 == a);
    REQUIRE(signal_data2 == OrderImbalance());
    REQUIRE(signal_data3 == OrderImbalance());
    model.insert(b);
    REQUIRE(signal_data1 == b);
    REQUIRE(signal_data2 == b);
    REQUIRE(signal_data3 == OrderImbalance());
    model.insert(c);
    REQUIRE(signal_data1 == c);
    REQUIRE(signal_data2 == c);
    REQUIRE(signal_data3 == c);
    model.insert(d);
    REQUIRE(signal_data1 == c);
    REQUIRE(signal_data2 == d);
    REQUIRE(signal_data3 == d);
    model.insert(e);
    REQUIRE(signal_data1 == c);
    REQUIRE(signal_data2 == d);
    REQUIRE(signal_data3 == e);
  }, "test_order_imbalance_signals");
}
