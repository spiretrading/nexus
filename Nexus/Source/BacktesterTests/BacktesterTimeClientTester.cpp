#include <doctest/doctest.h>
#include "Nexus/Backtester/BacktesterTimeClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct AdvanceEvent : BacktesterEvent {
    AdvanceEvent(ptime timestamp)
      : BacktesterEvent(timestamp) {}

    void execute() override {}
  };
}

TEST_SUITE("BacktesterTimeClient") {
  TEST_CASE("start_time") {
    auto start_time = time_from_string("2025-08-12 09:00:00.000");
    auto event_handler = BacktesterEventHandler(start_time);
    auto time_client = BacktesterTimeClient(Ref(event_handler));
    REQUIRE(time_client.get_time() == start_time);
    auto advance_time = start_time + minutes(1);
    auto event = std::make_shared<AdvanceEvent>(advance_time);
    event_handler.add(event);
    event->wait();
    REQUIRE(time_client.get_time() == advance_time);
  }
}
