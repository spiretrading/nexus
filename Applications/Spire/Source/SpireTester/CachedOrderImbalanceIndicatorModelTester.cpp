#include <catch.hpp>
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace boost;
using namespace boost::icl;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_imbalance(const std::string& symbol, const ptime& timestamp) {
    return OrderImbalance(Security(symbol, 0), Side::BID, 100,
      Money(Money::ONE), timestamp);
  }

  const auto A = make_imbalance("A", from_time_t(100));
  const auto B = make_imbalance("B", from_time_t(200));
  const auto C = make_imbalance("C", from_time_t(300));
  const auto D = make_imbalance("D", from_time_t(400));
  const auto E = make_imbalance("E", from_time_t(500));
}

TEST_CASE("test_local_publishing_subscribing",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto slot_data = OrderImbalance();
    model.subscribe([&] (const auto& imbalance) { slot_data = imbalance; });
    model.publish(A);
    REQUIRE(slot_data == A);
    model.publish(B);
    REQUIRE(slot_data == B);
    auto promise = model.load(TimeInterval::closed(from_time_t(100),
      from_time_t(200)));
    auto data = wait(std::move(promise));
    auto expected = std::vector<OrderImbalance>({A, B});
    REQUIRE(std::is_permutation(data.begin(), data.end(), expected.begin(),
      expected.end()));
  }, "test_local_publishing_subscribing");
}

TEST_CASE("test_local_subscribing_last_value",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto result1 = model.subscribe([=] (const auto& imbalance) {});
    auto snapshot1 = wait(std::move(result1.m_snapshot));
    REQUIRE(!snapshot1.is_initialized());
    model.publish(A);
    auto result2 = model.subscribe([=] (const auto& imbalance) {});
    auto snapshot2 = wait(std::move(result2.m_snapshot));
    REQUIRE(*snapshot2 == A);
    model.publish(B);
    auto result3 = model.subscribe([=] (const auto& imbalance) {});
    auto snapshot3 = wait(std::move(result3.m_snapshot));
    REQUIRE(*snapshot3 == B);
  }, "test_local_subscribing_last_value");
}

TEST_CASE("test_local_inserting_loading",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    model.insert(A);
    model.insert(B);
    model.insert(C);
    auto promise1 = model.load(TimeInterval::closed(from_time_t(100),
      from_time_t(300)));
    auto data1 = wait(std::move(promise1));
    auto expected1 = std::vector<OrderImbalance>({A, B, C});
    REQUIRE(std::is_permutation(data1.begin(), data1.end(), expected1.begin(),
      expected1.end()));
    model.insert(D);
    model.insert(E);
    auto promise2 = model.load(TimeInterval::closed(from_time_t(250),
      from_time_t(500)));
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({C, D, E});
    REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
      expected2.end()));
  }, "test_local_inserting_loading");
}

TEST_CASE("test_local_disconnection",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    auto slot_data1 = OrderImbalance();
    auto slot_data2 = OrderImbalance();
    model.subscribe([&] (const auto& imbalance) { slot_data1 = imbalance; });
    auto result = model.subscribe([&] (const auto& imbalance) {
      slot_data2 = imbalance; });
    result.m_connection.disconnect();
    model.publish(A);
    REQUIRE(slot_data1 == A);
    REQUIRE(slot_data2 == OrderImbalance());
  }, "test_local_disconnection");
}

TEST_CASE("test_local_out_of_order_inserting",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    model.insert(D);
    model.insert(B);
    model.insert(A);
    model.insert(C);
    auto promise = model.load(TimeInterval::closed(from_time_t(100),
      from_time_t(400)));
    auto data = wait(std::move(promise));
    auto expected = std::vector<OrderImbalance>({A, B, C, D});
    REQUIRE(std::is_permutation(data.begin(), data.end(), expected.begin(),
      expected.end()));
  }, "test_local_out_of_order_inserting");
}

TEST_CASE("test_local_loading_left_open_interval",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    model.insert(A);
    model.insert(B);
    model.insert(C);
    auto promise = model.load(continuous_interval<ptime>::left_open(
      from_time_t(100), from_time_t(300)));
    auto data = wait(std::move(promise));
    auto expected = std::vector<OrderImbalance>({B, C});
    REQUIRE(std::is_permutation(data.begin(), data.end(), expected.begin(),
      expected.end()));
  }, "test_local_loading_left_open_interval");
}

TEST_CASE("test_local_loading_right_open_interval",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    model.insert(A);
    model.insert(B);
    model.insert(C);
    auto promise = model.load(continuous_interval<ptime>::right_open(
      from_time_t(100), from_time_t(300)));
    auto data = wait(std::move(promise));
    auto expected = std::vector<OrderImbalance>({A, B});
    REQUIRE(std::is_permutation(data.begin(), data.end(), expected.begin(),
      expected.end()));
  }, "test_local_loading_right_open_interval");
}

TEST_CASE("test_local_loading_open_interval",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    model.insert(A);
    model.insert(B);
    model.insert(C);
    auto promise = model.load(continuous_interval<ptime>::open(
      from_time_t(100), from_time_t(300)));
    auto data = wait(std::move(promise));
    auto expected = std::vector<OrderImbalance>({B});
    REQUIRE(std::is_permutation(data.begin(), data.end(), expected.begin(),
      expected.end()));
  }, "test_local_loading_open_interval");
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
