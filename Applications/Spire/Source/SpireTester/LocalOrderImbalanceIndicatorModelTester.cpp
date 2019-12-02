#include <catch.hpp>
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace boost;
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
    auto promise = model.load({from_time_t(0), from_time_t(1000)});
    auto data = wait(std::move(promise));
    auto expected = std::vector<OrderImbalance>({A, B});
    REQUIRE(std::is_permutation(data.begin(), data.end(), expected.begin(),
      expected.end()));
  }, "test_local_publishing_subscribing");
}

TEST_CASE("test_local_inserting_loading",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = LocalOrderImbalanceIndicatorModel();
    model.insert(A);
    model.insert(B);
    model.insert(C);
    auto promise1 = model.load({from_time_t(0), from_time_t(1000)});
    auto data1 = wait(std::move(promise1));
    auto expected1 = std::vector<OrderImbalance>({A, B, C});
    REQUIRE(std::is_permutation(data.begin(), data.end(), expected1.begin(),
      expected1.end()));
    model.insert(D);
    model.insert(E);
    auto promise2 = model.load({from_time_t(250), from_time_t(1000)});
    auto data2 = wait(std::move(promise2));
    auto expected2 = std::vector<OrderImbalance>({C, D, E});
    REQUIRE(std::is_permutation(data.begin(), data.end(), expected2.begin(),
      expected2.end()));
  }, "test_local_inserting_loading");
}
