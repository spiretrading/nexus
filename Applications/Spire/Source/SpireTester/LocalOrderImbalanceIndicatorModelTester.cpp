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
    auto a = make("ABC", from_time_t(100));
    auto b = make("DEF", from_time_t(200));
    auto c = make("GHI", from_time_t(300));
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
    
  }, "test_subscribing_with_specific_range");
}

TEST_CASE("test_order_imbalance_signals",
    "[LocalOrderImbalanceIndicatorModel]") {
  run_test([] {
    
  }, "test_order_imbalance_signals");
}
