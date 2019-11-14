#include <catch.hpp>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
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

  const auto a = make("A", from_time_t(100));
  const auto b = make("B", from_time_t(200));
  const auto c = make("C", from_time_t(300));
  const auto d = make("D", from_time_t(400));
  const auto e = make("E", from_time_t(500));
}

TEST_CASE("test_basic_inserting_subscribing",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    model.insert(a);
    model.insert(b);
    model.insert(c);
    auto [connection, promise] = model.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data = wait(std::move(promise));
    auto expected = std::vector<OrderImbalance>({a, b, c});
    REQUIRE(std::is_permutation(data.begin(), data.end(), expected.begin(),
      expected.end()));
  }, "test_basic_inserting_subscribing");
}

TEST_CASE("test_subscribing_with_specific_range",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    model.insert(a);
    model.insert(b);
    model.insert(c);
    model.insert(d);
    model.insert(e);
    auto [connection1, promise1] = model.subscribe(from_time_t(100),
      from_time_t(300), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    auto expected1 = std::vector<OrderImbalance>({a, b, c});
    REQUIRE(std::is_permutation(data1.begin(), data1.end(), expected1.begin(),
      expected1.end()));
    auto [connection2, promise2] = model.subscribe(from_time_t(200),
      from_time_t(400), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({b, c, d});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
    auto [connection3, promise3] = model.subscribe(from_time_t(300),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    auto expected3 = std::vector<OrderImbalance>({c, d, e});
    REQUIRE(std::is_permutation(data3.begin(), data3.end(), expected3.begin(),
      expected3.end()));
  }, "test_subscribing_with_specific_range");
}

TEST_CASE("test_order_imbalance_signals",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
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
