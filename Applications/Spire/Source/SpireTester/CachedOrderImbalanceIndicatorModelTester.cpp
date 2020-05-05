#include <doctest/doctest.h>
#include "Spire/OrderImbalanceIndicator/CachedOrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/SpireTester/TestOrderImbalanceIndicatorModel.hpp"
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

  auto make_imbalance(const Security security, const ptime& timestamp) {
    return OrderImbalance(security, Side::BID, 100,
      Money(Money::ONE), timestamp);
  }

  const auto A = make_imbalance("A", from_time_t(100));
  const auto B = make_imbalance("B", from_time_t(200));
  const auto C = make_imbalance("C", from_time_t(300));
  const auto D = make_imbalance("D", from_time_t(400));
  const auto E = make_imbalance("E", from_time_t(500));

  const auto S = Security("A", 0);
  const auto F = make_imbalance(S, from_time_t(100));
  const auto G = make_imbalance(S, from_time_t(200));
  const auto H = make_imbalance(S, from_time_t(300));
  const auto I = make_imbalance(S, from_time_t(400));
  const auto J = make_imbalance(S, from_time_t(500));
}

TEST_SUITE("CachedOrderImbalanceIndicatorModel") {
  TEST_CASE("cached_publishing_subscribing") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
      auto slot_data = OrderImbalance();
      cached_model.subscribe([&] (const auto& imbalance) {
        slot_data = imbalance; });
      local_model->publish(A);
      REQUIRE(slot_data == A);
      local_model->publish(B);
      REQUIRE(slot_data == B);
      auto promise = cached_model.load(TimeInterval::closed(from_time_t(100),
        from_time_t(200)));
      auto data = wait(std::move(promise));
      REQUIRE(data == std::vector<OrderImbalance>({A, B}));
    });
  }

  TEST_CASE("cached_single_security_load_imbalances_pre_cached") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      local_model->insert(A);
      local_model->insert(B);
      local_model->insert(C);
      local_model->insert(D);
      local_model->insert(E);
      auto A2 = make_imbalance("A", from_time_t(200));
      auto A3 = make_imbalance("A", from_time_t(300));
      auto A4 = make_imbalance("A", from_time_t(400));
      local_model->insert(A2);
      local_model->insert(A3);
      local_model->insert(A4);
      auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
      auto promise1 = cached_model.load(TimeInterval::closed(from_time_t(0),
        from_time_t(1000)));
      wait(std::move(promise1));
      auto promise2 = cached_model.load(Security("A", 0), TimeInterval::closed(
        from_time_t(0), from_time_t(1000)));
      auto data = wait(std::move(promise2));
      REQUIRE(data == std::vector<OrderImbalance>({A, A2, A3, A4}));
    });
  }

  TEST_CASE("cached_subscribing_last_value") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
      auto result1 = cached_model.subscribe([=] (const auto& imbalance) {});
      auto snapshot1 = wait(std::move(result1.m_snapshot));
      REQUIRE(!snapshot1.is_initialized());
      local_model->publish(A);
      auto result2 = cached_model.subscribe([=] (const auto& imbalance) {});
      auto snapshot2 = wait(std::move(result2.m_snapshot));
      REQUIRE(*snapshot2 == A);
      local_model->publish(B);
      auto result3 = cached_model.subscribe([=] (const auto& imbalance) {});
      auto snapshot3 = wait(std::move(result3.m_snapshot));
      REQUIRE(*snapshot3 == B);
    });
  }

  TEST_CASE("cached_duplicate_timestamps_value") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
      local_model->publish(A);
      auto promise1 = cached_model.load(TimeInterval::closed(from_time_t(99),
        from_time_t(101)));
      auto data1 = wait(std::move(promise1));
      REQUIRE(data1 == std::vector<OrderImbalance>({A}));
      auto A2 = make_imbalance("A2", from_time_t(100));
      local_model->publish(A2);
      auto promise2 = cached_model.load(TimeInterval::closed(from_time_t(99),
        from_time_t(101)));
      auto data2 = wait(std::move(promise2));
      auto expected2 = std::vector<OrderImbalance>({A, A2});
      REQUIRE(std::is_permutation(data2.begin(), data2.end(), expected2.begin(),
        expected2.end()));
    });
  }

  TEST_CASE("cached_loading") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      local_model->insert(A);
      local_model->insert(B);
      local_model->insert(C);
      auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
      auto promise1 = cached_model.load(TimeInterval::closed(from_time_t(100),
        from_time_t(300)));
      auto data1 = wait(std::move(promise1));
      auto expected1 = std::vector<OrderImbalance>({A, B, C});
      REQUIRE(std::is_permutation(data1.begin(), data1.end(), expected1.begin(),
        expected1.end()));
      local_model->insert(D);
      local_model->insert(E);
      auto promise2 = cached_model.load(TimeInterval::closed(from_time_t(250),
        from_time_t(500)));
      auto data2 = wait(std::move(promise2));
      REQUIRE(data2 == std::vector<OrderImbalance>({C, D, E}));
    });
  }

  TEST_CASE("cached_disconnection") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
      auto slot_data1 = OrderImbalance();
      auto slot_data2 = OrderImbalance();
      cached_model.subscribe([&] (const auto& imbalance) {
        slot_data1 = imbalance; });
      auto result = cached_model.subscribe([&] (const auto& imbalance) {
        slot_data2 = imbalance; });
      result.m_connection.disconnect();
      local_model->publish(A);
      REQUIRE(slot_data1 == A);
      REQUIRE(slot_data2 == OrderImbalance());
    });
  }

  TEST_CASE("cached_out_of_order_inserting") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      local_model->insert(D);
      local_model->insert(B);
      local_model->insert(A);
      local_model->insert(C);
      auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
      auto promise = cached_model.load(TimeInterval::closed(from_time_t(100),
        from_time_t(400)));
      auto data = wait(std::move(promise));
      REQUIRE(data == std::vector<OrderImbalance>({A, B, C, D}));
    });
  }

  TEST_CASE("cached_loading_left_open_interval") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      local_model->insert(A);
      local_model->insert(B);
      local_model->insert(C);
      auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
      auto promise = cached_model.load(continuous_interval<ptime>::left_open(
        from_time_t(100), from_time_t(300)));
      auto data = wait(std::move(promise));
      REQUIRE(data == std::vector<OrderImbalance>({B, C}));
    });
  }

  TEST_CASE("cached_loading_right_open_interval") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      local_model->insert(A);
      local_model->insert(B);
      local_model->insert(C);
      auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
      auto promise = cached_model.load(continuous_interval<ptime>::right_open(
        from_time_t(100), from_time_t(300)));
      auto data = wait(std::move(promise));
      REQUIRE(data == std::vector<OrderImbalance>({A, B}));
    });
  }

  TEST_CASE("cached_loading_open_interval") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      local_model->insert(A);
      local_model->insert(B);
      local_model->insert(C);
      auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
      auto promise = cached_model.load(continuous_interval<ptime>::open(
        from_time_t(100), from_time_t(300)));
      auto data = wait(std::move(promise));
      REQUIRE(data == std::vector<OrderImbalance>({B}));
    });
  }

  TEST_CASE("cached_imbalance_right_no_overlap") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(TimeInterval::closed(
        from_time_t(100), from_time_t(200)));
      wait(test_model->pop_load())->set_result({A, B});
      wait(std::move(promise1));
      auto promise2 = cache_model.load(TimeInterval::closed(
        from_time_t(300), from_time_t(400)));
      auto request = wait(test_model->pop_load());
      REQUIRE(request->get_interval() == TimeInterval::closed(from_time_t(300),
        from_time_t(400)));
      request->set_result({});
    });
  }

  TEST_CASE("cached_imbalance_left_no_overlap") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(TimeInterval::closed(
        from_time_t(300), from_time_t(400)));
      wait(test_model->pop_load())->set_result({C, D});
      wait(std::move(promise1));
      auto promise2 = cache_model.load(TimeInterval::closed(
        from_time_t(100), from_time_t(200)));
      auto request = wait(test_model->pop_load());
      REQUIRE(request->get_interval() == TimeInterval::closed(from_time_t(100),
        from_time_t(200)));
      request->set_result({});
    });
  }

  TEST_CASE("cached_imbalance_right_overlap") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(TimeInterval::closed(
        from_time_t(100), from_time_t(300)));
      wait(test_model->pop_load())->set_result({A, B, C});
      wait(std::move(promise1));
      auto promise2 = cache_model.load(TimeInterval::closed(
        from_time_t(200), from_time_t(500)));
      auto request = wait(test_model->pop_load());
      REQUIRE(request->get_interval() == TimeInterval::closed(from_time_t(300),
        from_time_t(500)));
      request->set_result({});
    });
  }

  TEST_CASE("cached_imbalance_left_overlap") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(TimeInterval::closed(from_time_t(300),
        from_time_t(500)));
      wait(test_model->pop_load())->set_result({C, D, E});
      wait(std::move(promise1));
      auto promise2 = cache_model.load(TimeInterval::closed(from_time_t(100),
        from_time_t(400)));
      auto request = wait(test_model->pop_load());
      REQUIRE(request->get_interval() == TimeInterval::closed(from_time_t(100),
        from_time_t(300)));
      request->set_result({});
    });
  }

  TEST_CASE("cached_imbalance_superset") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(TimeInterval::closed(from_time_t(200),
        from_time_t(400)));
      wait(test_model->pop_load())->set_result({B, C, D});
      wait(std::move(promise1));
      auto promise2 = cache_model.load(TimeInterval::closed(from_time_t(100),
        from_time_t(500)));
      auto request1 = wait(test_model->pop_load());
      REQUIRE(request1->get_interval() == TimeInterval::closed(from_time_t(100),
        from_time_t(200)));
      request1->set_result({});
      auto request2 = wait(test_model->pop_load());
      REQUIRE(request2->get_interval() == TimeInterval::closed(from_time_t(400),
        from_time_t(500)));
      request2->set_result({});
    });
  }

  TEST_CASE("cached_imbalance_mixed_subsets_and_supersets") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(TimeInterval::closed(
        from_time_t(150), from_time_t(250)));
      wait(test_model->pop_load())->set_result({B});
      wait(std::move(promise1));
      auto promise2 = cache_model.load(TimeInterval::closed(
        from_time_t(350), from_time_t(450)));
      wait(test_model->pop_load())->set_result({D});
      wait(std::move(promise2));
      auto promise3 = cache_model.load(TimeInterval::closed(
        from_time_t(0), from_time_t(500)));
      auto request1 = wait(test_model->pop_load());
      REQUIRE(request1->get_interval() == TimeInterval::closed(from_time_t(0),
        from_time_t(150)));
      request1->set_result({});
      auto request2 = wait(test_model->pop_load());
      REQUIRE(request2->get_interval() == TimeInterval::closed(from_time_t(250),
        from_time_t(350)));
      request2->set_result({});
      auto request3 = wait(test_model->pop_load());
      REQUIRE(request3->get_interval() == TimeInterval::closed(from_time_t(450),
        from_time_t(500)));
      request3->set_result({});
    });
  }

  TEST_CASE("cached_imbalance_async_loads") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(TimeInterval::closed(from_time_t(100),
        from_time_t(300)));
      auto promise2 = cache_model.load(TimeInterval::closed(from_time_t(200),
        from_time_t(500)));
      REQUIRE(test_model->get_load_entry_count() == 2);
      auto load1 = wait(test_model->pop_load());
      auto load2 = wait(test_model->pop_load());
      REQUIRE(load1->get_interval() == TimeInterval::closed(from_time_t(100),
        from_time_t(300)));
      REQUIRE(load2->get_interval() == TimeInterval::closed(from_time_t(200),
        from_time_t(500)));
      load2->set_result({B, C, D, E});
      load1->set_result({A, B, C});
      wait(std::move(promise2));
      wait(std::move(promise1));
      auto promise3 = cache_model.load(TimeInterval::closed(
        from_time_t(100), from_time_t(500)));
      REQUIRE(test_model->get_load_entry_count() == 0);
      auto cached_data = wait(std::move(promise3));
      REQUIRE(cached_data == std::vector<OrderImbalance>({A, B, C, D, E}));
    });
  }

  TEST_CASE("cached_single_security_load") {
    run_test([] {
      auto local_model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      local_model->insert(A);
      local_model->insert(B);
      local_model->insert(C);
      local_model->insert(D);
      local_model->insert(E);
      auto A2 = make_imbalance("A", from_time_t(200));
      auto A3 = make_imbalance("A", from_time_t(300));
      auto A4 = make_imbalance("A", from_time_t(400));
      local_model->insert(A2);
      local_model->insert(A3);
      local_model->insert(A4);
      auto cached_model = CachedOrderImbalanceIndicatorModel(local_model);
      auto promise = cached_model.load(Security("A", 0), TimeInterval::closed(
        from_time_t(0), from_time_t(1000)));
      auto data = wait(std::move(promise));
      REQUIRE(data == std::vector<OrderImbalance>({A, A2, A3, A4}));
    });
  }

  TEST_CASE("cached_single_security_imbalance_right_no_overlap") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(S, TimeInterval::closed(
        from_time_t(100), from_time_t(200)));
      wait(test_model->pop_load())->set_result({F, G});
      wait(std::move(promise1));
      auto promise2 = cache_model.load(S, TimeInterval::closed(
        from_time_t(300), from_time_t(400)));
      auto request = wait(test_model->pop_load());
      REQUIRE(request->get_interval() == TimeInterval::closed(from_time_t(300),
        from_time_t(400)));
      request->set_result({});
    });
  }

  TEST_CASE("cached_single_security_imbalance_left_no_overlap") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(S, TimeInterval::closed(
        from_time_t(300), from_time_t(400)));
      wait(test_model->pop_load())->set_result({H, I});
      wait(std::move(promise1));
      auto promise2 = cache_model.load(S, TimeInterval::closed(
        from_time_t(100), from_time_t(200)));
      auto request = wait(test_model->pop_load());
      REQUIRE(request->get_interval() == TimeInterval::closed(from_time_t(100),
        from_time_t(200)));
      request->set_result({});
    });
  }

  TEST_CASE("cached_single_security_imbalance_right_overlap") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(S, TimeInterval::closed(
        from_time_t(100), from_time_t(300)));
      wait(test_model->pop_load())->set_result({F, G, H});
      wait(std::move(promise1));
      auto promise2 = cache_model.load(S, TimeInterval::closed(
        from_time_t(200), from_time_t(500)));
      auto request = wait(test_model->pop_load());
      REQUIRE(request->get_interval() == TimeInterval::closed(from_time_t(300),
        from_time_t(500)));
      request->set_result({});
    });
  }

  TEST_CASE("cached_single_security_imbalance_left_overlap") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(S, TimeInterval::closed(from_time_t(300),
        from_time_t(500)));
      wait(test_model->pop_load())->set_result({H, I, J});
      wait(std::move(promise1));
      auto promise2 = cache_model.load(S, TimeInterval::closed(from_time_t(100),
        from_time_t(400)));
      auto request = wait(test_model->pop_load());
      REQUIRE(request->get_interval() == TimeInterval::closed(from_time_t(100),
        from_time_t(300)));
      request->set_result({});
    });
  }

  TEST_CASE("cached_single_security_imbalance_superset") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(S, TimeInterval::closed(from_time_t(200),
        from_time_t(400)));
      wait(test_model->pop_load())->set_result({G, H, I});
      wait(std::move(promise1));
      auto promise2 = cache_model.load(S, TimeInterval::closed(from_time_t(100),
        from_time_t(500)));
      auto request1 = wait(test_model->pop_load());
      REQUIRE(request1->get_interval() == TimeInterval::closed(from_time_t(100),
        from_time_t(200)));
      request1->set_result({});
      auto request2 = wait(test_model->pop_load());
      REQUIRE(request2->get_interval() == TimeInterval::closed(from_time_t(400),
        from_time_t(500)));
      request2->set_result({});
    });
  }

  TEST_CASE("cached_single_security_imbalance_mixed_subsets_and_supersets") {
    run_test([] {
      auto test_model = std::make_shared<TestOrderImbalanceIndicatorModel>();
      auto cache_model = CachedOrderImbalanceIndicatorModel(test_model);
      auto promise1 = cache_model.load(S, TimeInterval::closed(
        from_time_t(150), from_time_t(250)));
      wait(test_model->pop_load())->set_result({G});
      wait(std::move(promise1));
      auto promise2 = cache_model.load(S, TimeInterval::closed(
        from_time_t(350), from_time_t(450)));
      wait(test_model->pop_load())->set_result({I});
      wait(std::move(promise2));
      auto promise3 = cache_model.load(S, TimeInterval::closed(
        from_time_t(0), from_time_t(500)));
      auto request1 = wait(test_model->pop_load());
      REQUIRE(request1->get_interval() == TimeInterval::closed(from_time_t(0),
        from_time_t(150)));
      request1->set_result({});
      auto request2 = wait(test_model->pop_load());
      REQUIRE(request2->get_interval() == TimeInterval::closed(from_time_t(250),
        from_time_t(350)));
      request2->set_result({});
      auto request3 = wait(test_model->pop_load());
      REQUIRE(request3->get_interval() == TimeInterval::closed(from_time_t(450),
        from_time_t(500)));
      request3->set_result({});
    });
  }
}
