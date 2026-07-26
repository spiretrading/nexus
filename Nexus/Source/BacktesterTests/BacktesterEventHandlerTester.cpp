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
    std::atomic_int* m_counter;

    ActiveEvent(ptime timestamp, std::atomic_int& counter)
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
    auto handler_range = BacktesterEventHandler(start, end);
    REQUIRE(handler_range.get_start_time() == start);
    REQUIRE(handler_range.get_end_time() == end);
    REQUIRE(handler_range.get_time() == start);
  }

  TEST_CASE("add") {
    auto handler =
      BacktesterEventHandler(time_from_string("2025-08-12 09:00:00.000"),
        time_from_string("2025-08-12 16:00:00.000"));
    auto active_count = std::atomic_int(0);
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
      BacktesterEventHandler(time_from_string("2025-08-12 09:00:00.000"),
        time_from_string("2025-08-12 16:00:00.000"));
    auto active_count = std::atomic_int(0);
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

  TEST_CASE("wait") {
    auto start = time_from_string("2025-08-12 09:00:00.000");
    auto end = time_from_string("2025-08-12 16:00:00.000");
    auto handler = BacktesterEventHandler(start, end);
    auto active_count = std::atomic_int(0);
    handler.add(std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 10:00:00.000"), active_count));
    handler.wait();
    REQUIRE(active_count == 1);
    REQUIRE(handler.get_time() >= end);
  }

  TEST_CASE("wait_fires_stranded_passive_event") {
    auto start = time_from_string("2025-08-12 09:00:00.000");
    auto end = time_from_string("2025-08-12 16:00:00.000");
    auto handler = BacktesterEventHandler(start, end);
    auto active_count = std::atomic_int(0);
    handler.add(std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 09:30:00.000"), active_count));
    auto passive_event = std::make_shared<PassiveEvent>(
      time_from_string("2025-08-12 12:00:00.000"));
    handler.add(passive_event);
    handler.wait();
    REQUIRE(passive_event->was_executed());
    REQUIRE(handler.get_time() >= end);
  }

  TEST_CASE("suspend_holds_events_until_resume") {
    auto handler =
      BacktesterEventHandler(time_from_string("2025-08-12 09:00:00.000"),
        time_from_string("2025-08-12 16:00:00.000"));
    auto active_count = std::atomic_int(0);
    handler.suspend();
    auto event = std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 10:00:00.000"), active_count);
    handler.add(event);
    REQUIRE(active_count == 0);
    REQUIRE(handler.get_time() ==
      time_from_string("2025-08-12 09:00:00.000"));
    handler.resume();
    event->wait();
    REQUIRE(active_count == 1);
    REQUIRE(handler.get_time() == time_from_string("2025-08-12 10:00:00.000"));
  }

  TEST_CASE("advance_processes_one_event_and_returns_it") {
    auto handler =
      BacktesterEventHandler(time_from_string("2025-08-12 09:00:00.000"),
        time_from_string("2025-08-12 16:00:00.000"));
    auto active_count = std::atomic_int(0);
    handler.suspend();
    auto event1 = std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 10:00:00.000"), active_count);
    auto event2 = std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 11:00:00.000"), active_count);
    handler.add(event2);
    handler.add(event1);
    auto stepped = handler.advance();
    REQUIRE(stepped == event1);
    REQUIRE(active_count == 1);
    REQUIRE(handler.get_time() == time_from_string("2025-08-12 10:00:00.000"));
    stepped = handler.advance();
    REQUIRE(stepped == event2);
    REQUIRE(active_count == 2);
    REQUIRE(handler.get_time() == time_from_string("2025-08-12 11:00:00.000"));
    REQUIRE(handler.advance() == nullptr);
    handler.resume();
  }

  TEST_CASE("advance_steps_passive_events") {
    auto handler =
      BacktesterEventHandler(time_from_string("2025-08-12 09:00:00.000"),
        time_from_string("2025-08-12 16:00:00.000"));
    handler.suspend();
    auto event = std::make_shared<PassiveEvent>(
      time_from_string("2025-08-12 09:30:00.000"));
    handler.add(event);
    auto stepped = handler.advance();
    REQUIRE(stepped == event);
    REQUIRE(event->was_executed());
    handler.resume();
  }

  TEST_CASE("advance_when_not_suspended") {
    auto handler =
      BacktesterEventHandler(time_from_string("2025-08-12 09:00:00.000"),
        time_from_string("2025-08-12 16:00:00.000"));
    auto active_count = std::atomic_int(0);
    auto event = std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 10:00:00.000"), active_count);
    handler.add(event);
    event->wait();
    REQUIRE(handler.advance() == nullptr);
  }

  TEST_CASE("suspend_mid_run_holds_subsequent_events") {
    auto handler =
      BacktesterEventHandler(time_from_string("2025-08-12 09:00:00.000"),
        time_from_string("2025-08-12 16:00:00.000"));
    auto active_count = std::atomic_int(0);
    auto event = std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 10:00:00.000"), active_count);
    handler.add(event);
    event->wait();
    handler.suspend();
    auto held = std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 11:00:00.000"), active_count);
    handler.add(held);
    REQUIRE(active_count == 1);
    REQUIRE(handler.get_time() == time_from_string("2025-08-12 10:00:00.000"));
    handler.resume();
    held->wait();
    REQUIRE(active_count == 2);
  }

  TEST_CASE("wait_when_active_event_already_processed") {
    auto start = time_from_string("2025-08-12 09:00:00.000");
    auto end = time_from_string("2025-08-12 16:00:00.000");
    auto handler = BacktesterEventHandler(start, end);
    auto active_count = std::atomic_int(0);
    auto event = std::make_shared<ActiveEvent>(
      time_from_string("2025-08-12 10:00:00.000"), active_count);
    handler.add(event);
    event->wait();
    REQUIRE(active_count == 1);
    handler.wait();
    REQUIRE(handler.get_time() >= end);
  }
}
