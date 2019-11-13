#include <catch.hpp>
#include "Spire/OrderImbalanceIndicator/CachedOrderImbalanceIndicatorModel.hpp"
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

  auto populated_local_model() {
    auto model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    model->insert(a);
    model->insert(b);
    model->insert(c);
    model->insert(d);
    model->insert(e);
    return model;
  }
}

TEST_CASE("cached_imbalance_test_basic_subscribing",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto cached_model = CachedOrderImbalanceIndicatorModel(
      populated_local_model());
    auto [connection1, promise1] = cached_model.subscribe(from_time_t(100),
      from_time_t(300), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    auto expected1 = std::vector<OrderImbalance>({a, b, c});
    REQUIRE(std::is_permutation(data1.begin(), data1.end(), expected1.begin(),
      expected1.end()));
    auto [connection2, promise2] = cached_model.subscribe(from_time_t(200),
      from_time_t(400), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({b, c, d});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
    auto [connection3, promise3] = cached_model.subscribe(from_time_t(300),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    auto expected3 = std::vector<OrderImbalance>({c, d, e});
    REQUIRE(std::is_permutation(data3.begin(), data3.end(), expected3.begin(),
      expected3.end()));
  }, "test_basic_subscribing");
}

TEST_CASE("test_empty_subscriptions", "[CachedOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto cached_model = CachedOrderImbalanceIndicatorModel(
      populated_local_model());
    auto [connection1, promise1] = cached_model.subscribe(from_time_t(1000),
      from_time_t(2000), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>());
    auto [connection2, promise2] = cached_model.subscribe(from_time_t(0),
      from_time_t(99), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>());
    auto [connection3, promise3] = cached_model.subscribe(from_time_t(101),
      from_time_t(199), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3 == std::vector<OrderImbalance>());
  }, "test_empty_subscriptions");
}

TEST_CASE("cached_imbalance_test_signals",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
    auto signal_data1 = OrderImbalance();
    auto signal_data2 = OrderImbalance();
    auto signal_data3 = OrderImbalance();
    auto signal_data4 = OrderImbalance();
    auto [connection1, promise1] = cached_model.subscribe(from_time_t(100),
      from_time_t(300), [&] (auto& i) { signal_data1 = i; });
    wait(std::move(promise1));
    auto [connection2, promise2] = cached_model.subscribe(from_time_t(200),
      from_time_t(400), [&] (auto& i) { signal_data2 = i; });
    wait(std::move(promise2));
    auto [connection3, promise3] = cached_model.subscribe(from_time_t(300),
      from_time_t(500), [&] (auto& i) { signal_data3 = i; });
    wait(std::move(promise3));
    auto [connection4, promise4] = cached_model.subscribe(from_time_t(900),
      from_time_t(1000), [&] (auto& i) { signal_data4 = i; });
    wait(std::move(promise4));
    local_model->insert(a);
    REQUIRE(signal_data1 == a);
    REQUIRE(signal_data2 == OrderImbalance());
    REQUIRE(signal_data3 == OrderImbalance());
    REQUIRE(signal_data4 == OrderImbalance());
    local_model->insert(b);
    REQUIRE(signal_data1 == b);
    REQUIRE(signal_data2 == b);
    REQUIRE(signal_data3 == OrderImbalance());
    REQUIRE(signal_data4 == OrderImbalance());
    local_model->insert(c);
    REQUIRE(signal_data1 == c);
    REQUIRE(signal_data2 == c);
    REQUIRE(signal_data3 == c);
    REQUIRE(signal_data4 == OrderImbalance());
    local_model->insert(d);
    REQUIRE(signal_data1 == c);
    REQUIRE(signal_data2 == d);
    REQUIRE(signal_data3 == d);
    REQUIRE(signal_data4 == OrderImbalance());
    local_model->insert(e);
    REQUIRE(signal_data1 == c);
    REQUIRE(signal_data2 == d);
    REQUIRE(signal_data3 == e);
    REQUIRE(signal_data4 == OrderImbalance());
  }, "test_signals");
}

TEST_CASE("cached_imbalance_test_right_no_overlap",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "imbalance_test_right_no_overlap");
}

TEST_CASE("cached_imbalance_test_left_no_overlap",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "imbalance_test_left_no_overlap");
}

TEST_CASE("cached_imbalance_test_right_overlap",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "imbalance_test_right_overlap");
}

TEST_CASE("cached_imbalance_test_left_overlap",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "imbalance_test_left_overlap");
}

TEST_CASE("cached_imbalance_test_subset",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "imbalance_test_subset");
}

TEST_CASE("cached_imbalance_test_single_superset",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "imbalance_test_single_superset");
}

TEST_CASE("cached_imbalance_test_multiple_supersets",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "imbalance_test_multiple_supersets");
}

TEST_CASE("cached_imbalance_test_multiple_subsets_and_supersets",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "imbalance_test_multiple_subsets_and_supersets");
}

TEST_CASE("cached_imbalance_test_multiple_cache_hits",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {
    
  }, "imbalance_test_multiple_cache_hits");
}
