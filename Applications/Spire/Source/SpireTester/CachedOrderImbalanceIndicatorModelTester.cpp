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

  auto make_local_model() {
    auto model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    model->insert(a);
    model->insert(b);
    model->insert(c);
    model->insert(d);
    model->insert(e);
    return model;
  }
}

TEST_CASE("test_basic_subscribing",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto cached_model = CachedOrderImbalanceIndicatorModel(make_local_model());
    auto [connection1, promise1] = cached_model.subscribe(from_time_t(100),
      from_time_t(300), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({a, b, c}));
    auto [connection2, promise2] = cached_model.subscribe(from_time_t(200),
      from_time_t(400), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({b, c, d}));
    auto [connection3, promise3] = cached_model.subscribe(from_time_t(300),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3 == std::vector<OrderImbalance>({c, d, e}));
  }, "test_basic_subscribing");
}

TEST_CASE("test_empty_subscriptions", "[CachedOrderImbalanceIndicatorModel]") {
  run_test([] {
    
  }, "test_empty_subscriptions");
}

TEST_CASE("test_signals",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([] {

  }, "test_signals");
}

TEST_CASE("test_right_no_overlap", "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "test_right_no_overlap");
}

TEST_CASE("test_left_no_overlap", "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "test_left_no_overlap");
}

TEST_CASE("test_right_overlap", "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "test_right_overlap");
}

TEST_CASE("test_left_overlap", "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "test_left_overlap");
}

TEST_CASE("test_subset", "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "test_subset");
}

TEST_CASE("test_single_superset", "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "test_single_superset");
}

TEST_CASE("test_multiple_supersets", "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "test_multiple_supersets");
}

TEST_CASE("test_multiple_subsets_and_supersets",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {

  }, "test_multiple_subsets_and_supersets");
}

TEST_CASE("test_multiple_cache_hits", "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {
    
  }, "test_multiple_cache_hits");
}
