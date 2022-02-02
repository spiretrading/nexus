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

  auto load_open(const auto& model, auto lower, auto upper) {
    auto result = model->subscribe(open(lower, upper), [] (const auto&) {});
    return wait(std::move(result.m_snapshot));
  }

  bool contains(const auto& container, const auto& value) {
    return
      std::find(container.begin(), container.end(), value) != container.end();
  }
}

TEST_SUITE("LocalOrderImbalanceIndicatorModel") {
  TEST_CASE("subscription_snapshot") {
    run_test([] {
      auto model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      auto A100 = make_imbalance("A", 100);
      auto B200 = make_imbalance("B", 200);
      model->publish(A100);
      model->publish(B200);
      auto load1 = load_closed(model, 0, 300);
      REQUIRE(load1.size() == 2);
      REQUIRE(contains(load1, A100));
      REQUIRE(contains(load1, B200));
      auto B300 = make_imbalance("B", 300);
      model->publish(B300);
      auto load2 = load_closed(model, 0, 300);
      REQUIRE(load2.size() == 2);
      REQUIRE(contains(load2, A100));
      REQUIRE(contains(load2, B300));
      auto load3 = load_open(model, 300, 400);
      REQUIRE(load3.empty());
      auto load4 = load_open(model, 100, 400);
      REQUIRE(load4.size() == 1);
      REQUIRE(load4.front() == B300);
      auto load5 = load_closed(model, 100, 250);
      REQUIRE(load5.size() == 2);
      REQUIRE(contains(load5, A100));
      REQUIRE(contains(load5, B200));
      auto load6 = load_open(model, 100, 300);
      REQUIRE(load6.size() == 1);
      REQUIRE(load6.front() == B200);
      auto load7 = load_open(model, 0, 100);
      REQUIRE(load7.empty());
      auto load8 = load_open(model, 300, 1000);
      REQUIRE(load8.empty());
    });
  }

  TEST_CASE("publishing") {
    auto model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    auto published1 = OrderImbalance();
    auto published2 = OrderImbalance();
    auto result1 = model->subscribe(closed(0, 1000),
      [&] (const auto& imbalance) { published1 = imbalance; });
    auto result2 = model->subscribe(open(100, 1000),
      [&] (const auto& imbalance) { published2 = imbalance; });
    REQUIRE(published1 == OrderImbalance());
    REQUIRE(published2 == OrderImbalance());
    auto A100 = make_imbalance("A", 100);
    model->publish(A100);
    REQUIRE(published1 == A100);
    REQUIRE(published2 == OrderImbalance());
    auto A200 = make_imbalance("A", 200);
    model->publish(A200);
    REQUIRE(published1 == A200);
    REQUIRE(published2 == A200);
    auto A1000 = make_imbalance("A", 1000);
    model->publish(A1000);
    REQUIRE(published1 == A1000);
    REQUIRE(published2 == A200);
  }

  TEST_CASE("publish_out_of_order") {
    run_test([] {
      auto model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
      auto A100 = make_imbalance("A", 100);
      auto A500 = make_imbalance("A", 500);
      model->publish(A500);
      model->publish(A100);
      auto load = load_closed(model, 0, 1000);
      REQUIRE(load.size() == 1);
      REQUIRE(load.front() == A500);
    });
  }

  TEST_CASE("publish_reentrant") {
    auto model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    auto published = std::vector<OrderImbalance>();
    auto subscription_publisher = model->subscribe(closed(0, 500),
      [&] (const auto& imbalance) {
        auto timestamp = to_time_t(imbalance.m_timestamp);
        if(timestamp % 100 == 0) {
          model->publish(make_imbalance("A", timestamp + 50));
        }
      });
    auto subscription = model->subscribe(closed(0, 500),
      [&] (const auto& imbalance) { published.push_back(imbalance); });
    auto A100 = make_imbalance("A", 100);
    auto A200 = make_imbalance("A", 200);
    auto A400 = make_imbalance("A", 400);
    model->publish(A100);
    model->publish(A200);
    model->publish(A400);
    REQUIRE(published.size() == 6);
    REQUIRE(published[0] == A100);
    REQUIRE(published[1] == make_imbalance("A", 150));
    REQUIRE(published[2] == A200);
    REQUIRE(published[3] == make_imbalance("A", 250));
    REQUIRE(published[4] == A400);
    REQUIRE(published[5] == make_imbalance("A", 450));
  }
}
