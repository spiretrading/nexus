#include <doctest/doctest.h>
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

TEST_SUITE("LocalOrderImbalanceIndicatorModel") {
  TEST_CASE("local_publishing_subscribing") {
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
    });
  }

  TEST_CASE("local_single_security_load") {
    run_test([] {
      auto model = LocalOrderImbalanceIndicatorModel();
      model.insert(A);
      model.insert(B);
      model.insert(C);
      model.insert(D);
      model.insert(E);
      auto A2 = make_imbalance("A", from_time_t(200));
      auto A3 = make_imbalance("A", from_time_t(300));
      auto A4 = make_imbalance("A", from_time_t(400));
      model.insert(A2);
      model.insert(A3);
      model.insert(A4);
      auto promise = model.load(Security("A", 0), TimeInterval::closed(
        from_time_t(0), from_time_t(1000)));
      auto data = wait(std::move(promise));
      REQUIRE(data == std::vector<OrderImbalance>({A, A2, A3, A4}));
    });
  }

  TEST_CASE("local_subscribing_last_value") {
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
    });
  }

  TEST_CASE("local_inserting_loading") {
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
    });
  }

  TEST_CASE("local_disconnection") {
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
    });
  }

  TEST_CASE("local_out_of_order_inserting") {
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
    });
  }

  TEST_CASE("local_loading_left_open_interval") {
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
    });
  }

  TEST_CASE("local_loading_right_open_interval") {
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
    });
  }

  TEST_CASE("local_loading_open_interval") {
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
    });
  }
}
