#include <future>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/Backtester/BacktesterEvent.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct TestEvent : BacktesterEvent {
    TestEvent(ptime timestamp)
      : BacktesterEvent(timestamp) {}

    void execute() override {
      complete();
    }
  };
}

TEST_SUITE("BacktesterEvent") {
  TEST_CASE("constructor") {
    auto timestamp = time_from_string("2025-08-12 12:34:56.000");
    auto event = TestEvent(timestamp);
    REQUIRE(event.get_timestamp() == timestamp);
    REQUIRE(!event.is_passive());
  }

  TEST_CASE("wait") {
    auto event = TestEvent(time_from_string("2025-08-12 12:34:56.000"));
    auto wait_async = std::async(std::launch::async, [&] {
      event.wait();
    });
    event.execute();
    wait_async.get();
  }
}
