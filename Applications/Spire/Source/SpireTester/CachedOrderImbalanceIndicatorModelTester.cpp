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

  const auto A = make("A", from_time_t(100));
  const auto B = make("B", from_time_t(200));
  const auto C = make("C", from_time_t(300));
  const auto D = make("D", from_time_t(400));
  const auto E = make("E", from_time_t(500));

  auto populated_local_model() {
    auto model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    model->insert(A);
    model->insert(B);
    model->insert(C);
    model->insert(D);
    model->insert(E);
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
    auto expected1 = std::vector<OrderImbalance>({A, B, C});
    REQUIRE(std::is_permutation(data1.begin(), data1.end(), expected1.begin(),
      expected1.end()));
    auto [connection2, promise2] = cached_model.subscribe(from_time_t(200),
      from_time_t(400), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({B, C, D});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
    auto [connection3, promise3] = cached_model.subscribe(from_time_t(300),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    auto expected3 = std::vector<OrderImbalance>({C, D, E});
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
    REQUIRE(signal_data2 == C);
    REQUIRE(signal_data3 == C);
    REQUIRE(signal_data4 == OrderImbalance());
    local_model->insert(D);
    REQUIRE(signal_data1 == C);
    REQUIRE(signal_data2 == D);
    REQUIRE(signal_data3 == D);
    REQUIRE(signal_data4 == OrderImbalance());
    local_model->insert(E);
    REQUIRE(signal_data1 == C);
    REQUIRE(signal_data2 == D);
    REQUIRE(signal_data3 == E);
    REQUIRE(signal_data4 == OrderImbalance());
  }, "cached_imbalance_test_signals");
}

TEST_CASE("cached_imbalance_test_signals_for_older_imbalances",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
    auto signal_data = OrderImbalance();
    auto [connection1, promise1] = cached_model.subscribe(from_time_t(100),
      from_time_t(100), [&] (auto& i) { signal_data = i; });
    wait(std::move(promise1));
    auto [connection2, promise2] = cached_model.subscribe(from_time_t(200),
      from_time_t(200), [&] (auto& i) { signal_data = i; });
    wait(std::move(promise2));
    auto [connection3, promise3] = cached_model.subscribe(from_time_t(300),
      from_time_t(300), [&] (auto& i) { signal_data = i; });
    wait(std::move(promise3));
    auto [connection4, promise4] = cached_model.subscribe(from_time_t(400),
      from_time_t(400), [&] (auto& i) { signal_data = i; });
    wait(std::move(promise4));
    auto [connection5, promise5] = cached_model.subscribe(from_time_t(500),
      from_time_t(500), [&] (auto& i) { signal_data = i; });
    wait(std::move(promise5));
    REQUIRE(signal_data == OrderImbalance());
    local_model->insert(E);
    REQUIRE(signal_data == E);
    local_model->insert(A);
    REQUIRE(signal_data == A);
    local_model->insert(B);
    REQUIRE(signal_data == B);
    local_model->insert(C);
    REQUIRE(signal_data == C);
    local_model->insert(D);
    REQUIRE(signal_data == D);
  }, "cached_imbalance_test_signals_for_older_imbalances");
}

TEST_CASE("cached_imbalance_test_right_no_overlap",
    "[CachedOrderImbalanceIndicatorModel]") {
  run_test([=] {
    auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
    auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
    auto [connection1, promise1] = cache_model.subscribe(from_time_t(100),
      from_time_t(200), [] (auto& i) {});
    wait(test_model->pop_subscription())->set_result({A, B});
    wait(std::move(promise1));
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(300),
      from_time_t(400), [] (auto& i) {});
    auto request = wait(test_model->pop_subscription());
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
    wait(test_model->pop_subscription())->set_result({C, D});
    wait(std::move(promise1));
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(100),
      from_time_t(200), [] (auto& i) {});
    auto request = wait(test_model->pop_subscription());
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
    wait(test_model->pop_subscription())->set_result({A, B, C});
    wait(std::move(promise1));
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(200),
      from_time_t(500), [] (auto& i) {});
    auto request = wait(test_model->pop_subscription());
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
    wait(test_model->pop_subscription())->set_result({C, D, E});
    wait(std::move(promise1));
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(100),
      from_time_t(400), [] (auto& i) {});
    auto request = wait(test_model->pop_subscription());
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
    wait(test_model->pop_subscription())->set_result({B, C, D});
    wait(std::move(promise1));
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(100),
      from_time_t(500), [] (auto& i) {});
    auto request1 = wait(test_model->pop_subscription());
    REQUIRE(request1->get_start() == from_time_t(100));
    REQUIRE(request1->get_end() == from_time_t(200));
    request1->set_result({});
    auto request2 = wait(test_model->pop_subscription());
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
    wait(test_model->pop_subscription())->set_result({B});
    wait(std::move(promise1));
    auto [connection2, promise2] = cache_model.subscribe(from_time_t(350),
      from_time_t(450), [] (auto& i) {});
    wait(test_model->pop_subscription())->set_result({D});
    wait(std::move(promise2));
    auto [connection3, promise3] = cache_model.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto request1 = wait(test_model->pop_subscription());
    REQUIRE(request1->get_start() == from_time_t(0));
    REQUIRE(request1->get_end() == from_time_t(150));
    request1->set_result({});
    auto request2 = wait(test_model->pop_subscription());
    REQUIRE(request2->get_start() == from_time_t(250));
    REQUIRE(request2->get_end() == from_time_t(350));
    request2->set_result({});
    auto request3 = wait(test_model->pop_subscription());
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
    REQUIRE(test_model->get_subscription_entry_count() == 2);
    auto subscribe1 = wait(test_model->pop_subscription());
    auto subscribe2 = wait(test_model->pop_subscription());
    REQUIRE(subscribe1->get_start() == from_time_t(100));
    REQUIRE(subscribe1->get_end() == from_time_t(300));
    REQUIRE(subscribe2->get_start() == from_time_t(200));
    REQUIRE(subscribe2->get_end() == from_time_t(500));
    subscribe2->set_result({B, C, D, E});
    subscribe1->set_result({A, B, C});
    wait(std::move(promise2));
    wait(std::move(promise1));
    auto [connection3, promise3] = cache_model.subscribe(from_time_t(100),
      from_time_t(500), [] (auto& i) {});
    REQUIRE(test_model->get_subscription_entry_count() == 0);
    auto cached_data = wait(std::move(promise3));
    auto expected = std::vector<OrderImbalance>({A, B, C, D, E});
    REQUIRE(std::is_permutation(cached_data.begin(), cached_data.end(),
      expected.begin(), expected.end()));
  }, "cached_imbalance_test_async_subscribes");
}
