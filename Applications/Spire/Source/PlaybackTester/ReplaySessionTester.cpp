#include <chrono>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include <QApplication>
#include "Spire/Playback/ReplaySession.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Spire;

namespace {
  const auto BASE = time_from_string("2024-06-01 10:00:00");
  const auto WAIT_LIMIT = std::chrono::seconds(5);
  const auto EVENT_INTERVAL_MS = 50;

  struct Fixture {
    FixedTimeClient m_time_client;
    std::shared_ptr<ReplaySession> m_session;

    Fixture(time_duration playhead, time_duration duration = hours(2))
      : m_time_client(BASE + hours(1)),
        m_session(std::make_shared<ReplaySession>(Timeline(BASE, duration),
          playhead, Decimal(1), TimeClient(&m_time_client))) {}

    void advance(time_duration duration) {
      m_time_client.set(m_time_client.get_time() + duration);
    }
  };

  template<typename F>
  void wait_until(F&& predicate) {
    auto deadline = std::chrono::steady_clock::now() + WAIT_LIMIT;
    while(!predicate()) {
      REQUIRE(std::chrono::steady_clock::now() < deadline);
      QApplication::processEvents(QEventLoop::AllEvents, EVENT_INTERVAL_MS);
    }
  }

  void process_events_for(std::chrono::milliseconds duration) {
    auto deadline = std::chrono::steady_clock::now() + duration;
    while(std::chrono::steady_clock::now() < deadline) {
      QApplication::processEvents(QEventLoop::AllEvents, EVENT_INTERVAL_MS);
    }
  }
}

TEST_SUITE("ReplaySession") {
  TEST_CASE("constructor") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& session = *fixture.m_session;
      REQUIRE(session.get_state()->get() == PlaybackState::PAUSED);
      REQUIRE(session.get_position()->get() == BASE + minutes(10));
      REQUIRE(session.get_timeline()->get().m_start == BASE);
      REQUIRE(session.get_timeline()->get().m_duration == hours(2));
      REQUIRE(session.get_playhead()->get() == minutes(10));
      REQUIRE(session.get_speed()->get() == Decimal(1));
    });
  }

  TEST_CASE("constructor_limits_the_playhead") {
    run_test([] {
      auto fixture = Fixture(hours(3));
      auto& session = *fixture.m_session;
      REQUIRE(session.get_playhead()->get() == hours(1));
      REQUIRE(session.get_position()->get() == BASE + hours(1));
      auto short_fixture = Fixture(hours(3), minutes(30));
      auto& short_session = *short_fixture.m_session;
      REQUIRE(short_session.get_playhead()->get() == minutes(30));
      REQUIRE(short_session.get_position()->get() == BASE + minutes(30));
      auto negative_fixture = Fixture(-minutes(5));
      auto& negative_session = *negative_fixture.m_session;
      REQUIRE(negative_session.get_playhead()->get() == minutes(0));
      REQUIRE(negative_session.get_position()->get() == BASE);
    });
  }

  TEST_CASE("state_and_position_are_read_only") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& session = *fixture.m_session;
      REQUIRE(session.get_state()->set(PlaybackState::REPLAYING) ==
        QValidator::State::Invalid);
      REQUIRE(session.get_state()->get() == PlaybackState::PAUSED);
      REQUIRE(session.get_position()->set(BASE) == QValidator::State::Invalid);
      REQUIRE(session.get_position()->get() == BASE + minutes(10));
    });
  }

  TEST_CASE("state_and_position_signal_their_changes") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& session = *fixture.m_session;
      auto states = std::vector<PlaybackState>();
      auto state_connection = session.get_state()->connect_update_signal(
        [&] (auto state) {
          states.push_back(state);
        });
      auto positions = std::vector<ptime>();
      auto position_connection =
        session.get_position()->connect_update_signal(
          [&] (const auto& position) {
            positions.push_back(position);
          });
      session.get_playhead()->set(minutes(20));
      REQUIRE(positions == std::vector<ptime>{BASE + minutes(20)});
      session.get_timeline()->set(Timeline(BASE, hours(3)));
      REQUIRE(positions.size() == 1);
      session.play();
      REQUIRE(states == std::vector<PlaybackState>{PlaybackState::REPLAYING});
      session.pause();
      REQUIRE(states == std::vector<PlaybackState>{
        PlaybackState::REPLAYING, PlaybackState::PAUSED});
    });
  }

  TEST_CASE("position_follows_the_playhead_and_the_timeline") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& session = *fixture.m_session;
      session.get_playhead()->set(minutes(20));
      REQUIRE(session.get_position()->get() == BASE + minutes(20));
      session.get_timeline()->set(Timeline(BASE - minutes(5), hours(2)));
      REQUIRE(session.get_playhead()->get() == minutes(20));
      REQUIRE(session.get_position()->get() == BASE + minutes(15));
    });
  }

  TEST_CASE("setting_the_playhead_is_limited_to_the_present") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& session = *fixture.m_session;
      REQUIRE(session.get_playhead()->set(hours(2)) ==
        QValidator::State::Acceptable);
      REQUIRE(session.get_playhead()->get() == hours(1));
      REQUIRE(session.get_position()->get() == BASE + hours(1));
      REQUIRE(session.get_state()->get() == PlaybackState::PAUSED);
      auto playing_fixture = Fixture(minutes(10));
      auto& playing_session = *playing_fixture.m_session;
      playing_session.play();
      REQUIRE(playing_session.get_state()->get() == PlaybackState::REPLAYING);
      REQUIRE(playing_session.get_playhead()->set(hours(2)) ==
        QValidator::State::Acceptable);
      REQUIRE(playing_session.get_playhead()->get() == hours(1));
      REQUIRE(playing_session.get_state()->get() == PlaybackState::REAL_TIME);
    });
  }

  TEST_CASE("setting_the_playhead_is_limited_to_the_timeline") {
    run_test([] {
      auto fixture = Fixture(minutes(10), minutes(30));
      auto& session = *fixture.m_session;
      session.get_playhead()->set(hours(1));
      REQUIRE(session.get_playhead()->get() == minutes(30));
      session.get_playhead()->set(-minutes(5));
      REQUIRE(session.get_playhead()->get() == minutes(0));
    });
  }

  TEST_CASE("playhead_reports_its_range") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& playhead = *fixture.m_session->get_playhead();
      REQUIRE(playhead.get_minimum() == time_duration(minutes(0)));
      REQUIRE(playhead.get_maximum() == time_duration(hours(1)));
      fixture.advance(minutes(30));
      REQUIRE(playhead.get_maximum() == hours(1) + minutes(30));
      fixture.m_session->get_timeline()->set(Timeline(BASE, minutes(30)));
      REQUIRE(playhead.get_maximum() == time_duration(minutes(30)));
    });
  }

  TEST_CASE("setting_the_timeline_limits_the_playhead") {
    run_test([] {
      auto fixture = Fixture(minutes(50));
      auto& session = *fixture.m_session;
      session.get_timeline()->set(Timeline(BASE + minutes(30), hours(2)));
      REQUIRE(session.get_playhead()->get() == minutes(30));
      REQUIRE(session.get_position()->get() == BASE + hours(1));
      session.get_timeline()->set(Timeline(BASE, minutes(20)));
      REQUIRE(session.get_playhead()->get() == minutes(20));
      REQUIRE(session.get_position()->get() == BASE + minutes(20));
      REQUIRE(session.get_state()->get() == PlaybackState::PAUSED);
      auto playing_fixture = Fixture(minutes(50));
      auto& playing_session = *playing_fixture.m_session;
      playing_session.play();
      REQUIRE(playing_session.get_state()->get() == PlaybackState::REPLAYING);
      playing_session.get_timeline()->set(
        Timeline(BASE + minutes(30), hours(2)));
      REQUIRE(playing_session.get_playhead()->get() == minutes(30));
      REQUIRE(playing_session.get_state()->get() == PlaybackState::REAL_TIME);
    });
  }

  TEST_CASE("speed_must_be_positive") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& session = *fixture.m_session;
      REQUIRE(session.get_speed()->set(Decimal(0)) ==
        QValidator::State::Invalid);
      REQUIRE(session.get_speed()->set(Decimal(-1)) ==
        QValidator::State::Invalid);
      REQUIRE(session.get_speed()->get() == Decimal(1));
      REQUIRE(session.get_speed()->set(Decimal(2)) ==
        QValidator::State::Acceptable);
      REQUIRE(session.get_speed()->get() == Decimal(2));
      auto invalid_session = ReplaySession(Timeline(BASE, hours(2)),
        minutes(10), Decimal(0), TimeClient(&fixture.m_time_client));
      REQUIRE(invalid_session.get_speed()->get() == Decimal(1));
    });
  }

  TEST_CASE("play_and_pause") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& session = *fixture.m_session;
      session.play();
      REQUIRE(session.get_state()->get() == PlaybackState::REPLAYING);
      session.pause();
      REQUIRE(session.get_state()->get() == PlaybackState::PAUSED);
      fixture.advance(seconds(5));
      process_events_for(std::chrono::milliseconds(300));
      REQUIRE(session.get_playhead()->get() == minutes(10));
    });
  }

  TEST_CASE("playing_advances_the_playhead_at_the_speed") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& session = *fixture.m_session;
      session.get_speed()->set(Decimal(2));
      session.play();
      fixture.advance(seconds(5));
      wait_until([&] {
        return session.get_playhead()->get() == minutes(10) + seconds(10);
      });
      REQUIRE(session.get_position()->get() ==
        BASE + minutes(10) + seconds(10));
      REQUIRE(session.get_state()->get() == PlaybackState::REPLAYING);
    });
  }

  TEST_CASE("changing_the_speed_while_playing_keeps_the_playhead") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& session = *fixture.m_session;
      session.play();
      fixture.advance(seconds(5));
      wait_until([&] {
        return session.get_playhead()->get() == minutes(10) + seconds(5);
      });
      session.get_speed()->set(Decimal(2));
      fixture.advance(seconds(5));
      wait_until([&] {
        return session.get_playhead()->get() == minutes(10) + seconds(15);
      });
    });
  }

  TEST_CASE("playing_at_the_present_is_real_time") {
    run_test([] {
      auto fixture = Fixture(hours(1));
      auto& session = *fixture.m_session;
      session.play();
      REQUIRE(session.get_state()->get() == PlaybackState::REAL_TIME);
      fixture.advance(seconds(5));
      wait_until([&] {
        return session.get_playhead()->get() == hours(1) + seconds(5);
      });
      REQUIRE(session.get_state()->get() == PlaybackState::REAL_TIME);
    });
  }

  TEST_CASE("reaching_and_leaving_the_present") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& session = *fixture.m_session;
      session.play();
      REQUIRE(session.get_state()->get() == PlaybackState::REPLAYING);
      session.get_playhead()->set(hours(1));
      REQUIRE(session.get_state()->get() == PlaybackState::REAL_TIME);
      session.get_playhead()->set(minutes(30));
      REQUIRE(session.get_state()->get() == PlaybackState::REPLAYING);
    });
  }

  TEST_CASE("seeking_while_playing_continues_from_the_new_playhead") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& session = *fixture.m_session;
      session.play();
      session.get_playhead()->set(minutes(30));
      fixture.advance(seconds(5));
      wait_until([&] {
        return session.get_playhead()->get() == minutes(30) + seconds(5);
      });
      REQUIRE(session.get_state()->get() == PlaybackState::REPLAYING);
    });
  }

  TEST_CASE("seeking_while_paused_stays_paused") {
    run_test([] {
      auto fixture = Fixture(minutes(10));
      auto& session = *fixture.m_session;
      session.get_playhead()->set(hours(1));
      REQUIRE(session.get_state()->get() == PlaybackState::PAUSED);
    });
  }

  TEST_CASE("pauses_at_the_end_of_a_past_timeline") {
    run_test([] {
      auto fixture = Fixture(seconds(9), seconds(10));
      auto& session = *fixture.m_session;
      session.play();
      REQUIRE(session.get_state()->get() == PlaybackState::REPLAYING);
      fixture.advance(seconds(5));
      wait_until([&] {
        return session.get_state()->get() == PlaybackState::PAUSED;
      });
      REQUIRE(session.get_playhead()->get() == seconds(10));
      session.play();
      REQUIRE(session.get_state()->get() == PlaybackState::PAUSED);
    });
  }
}
