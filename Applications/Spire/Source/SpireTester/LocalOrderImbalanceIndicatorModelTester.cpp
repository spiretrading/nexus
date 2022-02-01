#include <doctest/doctest.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_imbalance(const auto& symbol, auto timestamp) {
    return OrderImbalance(Security(symbol, DefaultCountries::US()), Side::ASK,
      Quantity(100), Money(12.34), from_time_t(timestamp));
  }

  auto closed(auto lower, auto upper) {
    return TimeInterval::closed(from_time_t(lower), from_time_t(upper));
  }

  auto open(auto lower, auto upper) {
    return TimeInterval::open(from_time_t(lower), from_time_t(upper));
  }

  auto load_closed(const auto& model, auto lower, auto upper) {
    auto result = model->subscribe(closed(lower, upper), [] (const auto&) {});
    return wait(std::move(result.m_snapshot));
  }
}

TEST_SUITE("LocalOrderImbalanceIndicatorModel") {
  TEST_CASE("subscription_result") {
    run_test([] {
      auto model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      auto A100 = make_imbalance("A", 100);
      auto B200 = make_imbalance("B", 200);
      model->publish(A100);
      model->publish(B200);
      auto load1 = load_closed(model, 0, 300);
      REQUIRE(load1.size() == 2);
    });
  }

  TEST_CASE("publishing") {
    
  }

  TEST_CASE("multiple_subscriptions") {
    
  }

  TEST_CASE("publish_reentrant") {
    
  }

  TEST_CASE("delayed_subscription_result") {
    
  }
}
