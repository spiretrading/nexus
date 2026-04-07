#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <doctest/doctest.h>
#include "Nexus/Backtester/ActiveBacktesterEvent.hpp"
#include "Nexus/Backtester/BacktesterTimer.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("BacktesterTimer") {
  TEST_CASE("expiry") {
    auto start_time = time_from_string("2016-05-06 00:00:00");
    auto event_handler = BacktesterEventHandler(start_time);
    auto timer = BacktesterTimer(seconds(1), Ref(event_handler));
    auto routines = RoutineTaskQueue();
    auto expected_timestamp = start_time + seconds(1);
    auto query_complete_mutex = Mutex();
    auto test_complete_condition = ConditionVariable();
    auto test_succeeded = optional<bool>();
    timer.get_publisher().monitor(
      routines.get_slot<Timer::Result>([&] (auto result) {
        auto lock = std::lock_guard(query_complete_mutex);
        auto timestamp = event_handler.get_time();
        test_succeeded = timestamp == expected_timestamp &&
          result == Timer::Result::EXPIRED;
        test_complete_condition.notify_one();
      }));
    timer.start();
    event_handler.add(std::make_shared<ActiveBacktesterEvent>(
      time_from_string("2016-05-07 00:00:00")));
    auto lock = std::unique_lock(query_complete_mutex);
    while(!test_succeeded) {
      test_complete_condition.wait(lock);
    }
    REQUIRE(*test_succeeded);
  }

  TEST_CASE("cancel") {
    auto start_time = time_from_string("2016-05-06 00:00:00");
    auto event_handler = BacktesterEventHandler(start_time);
    auto timer_a = BacktesterTimer(seconds(1), Ref(event_handler));
    auto timer_b = BacktesterTimer(seconds(2), Ref(event_handler));
    auto routines = RoutineTaskQueue();
    auto expected_timestamp = start_time + seconds(1);
    auto query_complete_mutex = Mutex();
    auto test_complete_condition = ConditionVariable();
    auto test_succeeded = optional<bool>();
    timer_a.get_publisher().monitor(
      routines.get_slot<Timer::Result>([&] (Timer::Result result) {
        auto timestamp = event_handler.get_time();
        REQUIRE(timestamp == expected_timestamp);
        REQUIRE(result == Timer::Result::EXPIRED);
        timer_b.cancel();
      }));
    timer_b.get_publisher().monitor(
      routines.get_slot<Timer::Result>([&] (auto result) {
        auto timestamp = event_handler.get_time();
        REQUIRE(timestamp == expected_timestamp);
        REQUIRE(result == Timer::Result::CANCELED);
        auto lock = std::lock_guard(query_complete_mutex);
        test_succeeded = true;
        test_complete_condition.notify_one();
      }));
    routines.push([&] {
      timer_b.start();
      timer_a.start();
      event_handler.add(std::make_shared<ActiveBacktesterEvent>(
        time_from_string("2016-05-07 00:00:00")));
    });
    auto lock = std::unique_lock(query_complete_mutex);
    while(!test_succeeded) {
      test_complete_condition.wait(lock);
    }
    REQUIRE(*test_succeeded);
  }
}
