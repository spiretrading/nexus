#include <catch.hpp>
#include "Spire/OrderImbalanceIndicator/CachedOrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/SpireTester/TestOrderImbalanceIndicatorModel.hpp"

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
  }, "cached_imbalance_test_basic_subscribing");
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
  }, "cached_imbalance_test_empty_subscriptions");
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
  }, "cached_imbalance_test_signals");
}

TEST_CASE("cached_imbalance_test_right_no_overlap",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {
    auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
    auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
    auto [connection1, promise1] = cache_model.subscribe(from_time_t(100),
      from_time_t(200), [] (auto& i) {});
    wait(test_model->pop_subscribe())->set_result({a, b});
    wait(std::move(promise1));
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(300),
      from_time_t(400), [] (auto& i) {});
    auto request = wait(test_model->pop_subscribe());
    REQUIRE(request->get_start() == from_time_t(300));
    REQUIRE(request->get_end() == from_time_t(400));
    request->set_result({});
  }, "cached_imbalance_test_right_no_overlap");
}

TEST_CASE("cached_imbalance_test_left_no_overlap",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {
    auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
    auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
    auto [connection1, promise1] = cache_model.subscribe(from_time_t(300),
      from_time_t(400), [] (auto& i) {});
    wait(test_model->pop_subscribe())->set_result({c, d});
    wait(std::move(promise1));
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(100),
      from_time_t(200), [] (auto& i) {});
    auto request = wait(test_model->pop_subscribe());
    REQUIRE(request->get_start() == from_time_t(100));
    REQUIRE(request->get_end() == from_time_t(200));
    request->set_result({});
  }, "cached_imbalance_test_left_no_overlap");
}

TEST_CASE("cached_imbalance_test_right_overlap",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {
    auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
    auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
    auto [connection1, promise1] = cache_model.subscribe(from_time_t(100),
      from_time_t(300), [] (auto& i) {});
    wait(test_model->pop_subscribe())->set_result({a, b, c});
    wait(std::move(promise1));
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(200),
      from_time_t(500), [] (auto& i) {});
    auto request = wait(test_model->pop_subscribe());
    REQUIRE(request->get_start() == from_time_t(300));
    REQUIRE(request->get_end() == from_time_t(500));
    request->set_result({});
  }, "cached_imbalance_test_right_overlap");
}

TEST_CASE("cached_imbalance_test_left_overlap",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {
    auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
    auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
    auto [connection1, promise1] = cache_model.subscribe(from_time_t(300),
      from_time_t(500), [] (auto& i) {});
    wait(test_model->pop_subscribe())->set_result({c, d, e});
    wait(std::move(promise1));
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(100),
      from_time_t(400), [] (auto& i) {});
    auto request = wait(test_model->pop_subscribe());
    REQUIRE(request->get_start() == from_time_t(100));
    REQUIRE(request->get_end() == from_time_t(300));
    request->set_result({});
  }, "cached_imbalance_test_left_overlap");
}

TEST_CASE("cached_imbalance_test_superset",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {
    auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
    auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
    auto [connection1, promise1] = cache_model.subscribe(from_time_t(200),
      from_time_t(400), [] (auto& i) {});
    wait(test_model->pop_subscribe())->set_result({b, c, d});
    wait(std::move(promise1));
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(100),
      from_time_t(500), [] (auto& i) {});
    auto request1 = wait(test_model->pop_subscribe());
    REQUIRE(request1->get_start() == from_time_t(100));
    REQUIRE(request1->get_end() == from_time_t(200));
    request1->set_result({});
    auto request2 = wait(test_model->pop_subscribe());
    REQUIRE(request2->get_start() == from_time_t(400));
    REQUIRE(request2->get_end() == from_time_t(500));
    request2->set_result({});
  }, "cached_imbalance_test_superset");
}

TEST_CASE("cached_imbalance_test_mixed_subsets_and_supersets",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {
    auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
    auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
    auto [connection1, promise1] = cache_model.subscribe(from_time_t(150),
      from_time_t(250), [] (auto& i) {});
    wait(test_model->pop_subscribe())->set_result({b});
    wait(std::move(promise1));
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(350),
      from_time_t(450), [] (auto& i) {});
    wait(test_model->pop_subscribe())->set_result({d});
    wait(std::move(promise2));
    auto [connection3, promise3] = cache_model.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto request1 = wait(test_model->pop_subscribe());
    REQUIRE(request1->get_start() == from_time_t(0));
    REQUIRE(request1->get_end() == from_time_t(150));
    request1->set_result({});
    auto request2 = wait(test_model->pop_subscribe());
    REQUIRE(request2->get_start() == from_time_t(250));
    REQUIRE(request2->get_end() == from_time_t(350));
    request2->set_result({});
    auto request3 = wait(test_model->pop_subscribe());
    REQUIRE(request3->get_start() == from_time_t(450));
    REQUIRE(request3->get_end() == from_time_t(500));
    request3->set_result({});
  }, "cached_imbalance_test_mixed_subsets_and_supersets");
}

TEST_CASE("cached_imbalance_test_async_subscribes",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {
    auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
    auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
    auto [connection1, promise1] = cache_model.subscribe(from_time_t(100),
      from_time_t(300), [] (auto& i) {});
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(200),
      from_time_t(500), [] (auto& i) {});
    REQUIRE(test_model->get_subscribe_entry_count() == 2);
    auto subscribe1 = wait(test_model->pop_subscribe());
    auto subscribe2 = wait(test_model->pop_subscribe());
    REQUIRE(subscribe1->get_start() == from_time_t(100));
    REQUIRE(subscribe1->get_end() == from_time_t(300));
    REQUIRE(subscribe2->get_start() == from_time_t(200));
    REQUIRE(subscribe2->get_end() == from_time_t(500));
    subscribe2->set_result({b, c, d, e});
    subscribe1->set_result({a, b, c});
    wait(std::move(promise2));
    wait(std::move(promise1));
    auto [connection3, promise3] = cache_model.subscribe(from_time_t(100),
      from_time_t(500), [] (auto& i) {});
    auto cached_data = wait(std::move(promise3));
    auto expected = std::vector<OrderImbalance>({a, b, c, d, e});
    REQUIRE(test_model->get_subscribe_entry_count() == 0);
    REQUIRE(std::is_permutation(cached_data.begin(), cached_data.end(),
      expected.begin(), expected.end()));
  }, "cached_imbalance_test_async_subscribes");
}
