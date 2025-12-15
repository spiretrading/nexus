#include <doctest/doctest.h>
#include "Nexus/Backtester/BacktesterEventHandler.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct PassiveEvent : BacktesterEvent {
    bool m_executed;

    PassiveEvent(ptime timestamp)
      : BacktesterEvent(timestamp),
        m_executed(false) {}

    bool is_passive() const override {
      return true;
    }

    void execute() override {
      m_executed = true;
      complete();
    }

    bool was_executed() const {
      return m_executed;
    }
  };

  struct ActiveEvent : BacktesterEvent {
    std::atomic<int>* m_counter;

    ActiveEvent(ptime timestamp, std::atomic<int>& counter)
      : BacktesterEvent(timestamp),
        m_counter(&counter) {}

    void execute() override {
      ++*m_counter;
      complete();
    }
  };
}

TEST_SUITE("BacktesterEventHandler") {
  TEST_CASE("constructor") {
    auto start = time_from_string("2025-08-12 09:00:00.000");
    auto end = time_from_string("2025-08-12 16:00:00.000");
    auto handler_single = BacktesterEventHandler(start);
    REQUIRE(handler_single.get_start_time() == start);
    REQUIRE(handler_single.get_end_time() == pos_infin);
    REQUIRE(handler_single.get_time() == start);
    auto handler_range = BacktesterEventHandler(start, end);
    REQUIRE(handler_range.get_start_time() == start);
    REQUIRE(handler_range.get_end_time() == end);
    REQUIRE(handler_range.get_time() == start);
  }

  TEST_CASE("add") {
    auto handler =
      BacktesterEventHandler(time_from_string("2025-08-12 09:00:00.000"));
    auto active_count = std::atomic<int>(0);
    auto passive_event = std::make_shared<PassiveEvent>(
      time_from_string("2025-08-12 09:30:00.000"));
    handler.add(passive_event);
    auto active_event1 = std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 10:00:00.000"), active_count);
    handler.add(active_event1);
    auto active_event2 = std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 11:00:00.000"), active_count);
    handler.add(active_event2);
    active_event1->wait();
    active_event2->wait();
    passive_event->wait();
    REQUIRE(passive_event->was_executed());
    REQUIRE(active_count == 2);
    REQUIRE(handler.get_time() == time_from_string("2025-08-12 11:00:00.000"));
  }

  TEST_CASE("add_events") {
    auto handler =
      BacktesterEventHandler(time_from_string("2025-08-12 09:00:00.000"));
    auto active_count = std::atomic<int>(0);
    auto passive_event1 = std::make_shared<PassiveEvent>(
      time_from_string("2025-08-12 09:15:00.000"));
    auto active_event1 = std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 09:30:00.000"), active_count);
    auto passive_event2 = std::make_shared<PassiveEvent>(
      time_from_string("2025-08-12 09:45:00.000"));
    auto active_event2 = std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 10:00:00.000"), active_count);
    auto events = std::vector<std::shared_ptr<BacktesterEvent>>{
      passive_event2, active_event2, passive_event1, active_event1
    };
    handler.add(events);
    active_event1->wait();
    active_event2->wait();
    passive_event1->wait();
    passive_event2->wait();
    REQUIRE(passive_event1->was_executed());
    REQUIRE(passive_event2->was_executed());
    REQUIRE(active_count == 2);
    REQUIRE(handler.get_time() == time_from_string("2025-08-12 10:00:00.000"));
  }
}
