#ifndef NEXUS_BACKTESTER_EVENT_HANDLER_HPP
#define NEXUS_BACKTESTER_EVENT_HANDLER_HPP
#include <algorithm>
#include <deque>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/LockRelease.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/TimeServiceTests/TestTimeClient.hpp>
#include <Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Backtester/BacktesterEvent.hpp"

namespace Nexus {

  /** Implements an event loop to handle BacktesterEvents. */
  class BacktesterEventHandler {
    public:

      /**
       * Constructs a BacktesterEventHandler.
       * @param start The starting point of the backtester.
       */
      explicit BacktesterEventHandler(boost::posix_time::ptime start);

      /**
       * Constructs a BacktesterEventHandler.
       * @param start The starting point of the backtester.
       * @param end The time to stop backtesting.
       */
      BacktesterEventHandler(
        boost::posix_time::ptime start, boost::posix_time::ptime end);

      ~BacktesterEventHandler();

      /** Returns the start time. */
      boost::posix_time::ptime get_start_time() const;

      /** Returns the end time. */
      boost::posix_time::ptime get_end_time() const;

      /** Returns the current time in the backtester. */
      boost::posix_time::ptime get_time() const;

      /**
       * Adds an event to be handled.
       * @param event The event to handle.
       */
      void add(const std::shared_ptr<BacktesterEvent>& event);

      /**
       * Adds a list of events to be handled.
       * @param events The list of events to handle.
       */
      void add(const std::vector<std::shared_ptr<BacktesterEvent>>& events);

      void close();

    private:
      mutable Beam::Threading::Mutex m_mutex;
      boost::posix_time::ptime m_start_time;
      boost::posix_time::ptime m_end_time;
      Beam::TimeService::Tests::TimeServiceTestEnvironment m_time_environment;
      std::deque<std::shared_ptr<BacktesterEvent>> m_events;
      std::size_t m_active_count;
      Beam::Threading::ConditionVariable m_event_available_condition;
      Beam::Routines::RoutineHandler m_event_loop_routine;
      Beam::IO::OpenState m_open_state;

      BacktesterEventHandler(const BacktesterEventHandler&) = delete;
      BacktesterEventHandler& operator =(
        const BacktesterEventHandler&) = delete;
      void event_loop();
  };

  inline BacktesterEventHandler::BacktesterEventHandler(
    boost::posix_time::ptime start)
    : BacktesterEventHandler(start, boost::posix_time::pos_infin) {}

  inline BacktesterEventHandler::BacktesterEventHandler(
      boost::posix_time::ptime start, boost::posix_time::ptime end)
      : m_start_time(start),
        m_end_time(end),
        m_active_count(0),
        m_time_environment(m_start_time) {
    try {
      m_event_loop_routine = Beam::Routines::Spawn(
        std::bind_front(&BacktesterEventHandler::event_loop, this));
    } catch(const std::exception&) {
      close();
      BOOST_RETHROW;
    }
  }

  inline BacktesterEventHandler::~BacktesterEventHandler() {
    close();
  }

  inline boost::posix_time::ptime
      BacktesterEventHandler::get_start_time() const {
    return m_start_time;
  }

  inline boost::posix_time::ptime BacktesterEventHandler::get_end_time() const {
    return m_end_time;
  }

  inline boost::posix_time::ptime BacktesterEventHandler::get_time() const {
    return m_time_environment.GetTime();
  }

  inline void BacktesterEventHandler::add(
      const std::shared_ptr<BacktesterEvent>& event) {
    auto is_active = !event->is_passive();
    {
      auto lock = std::lock_guard(m_mutex);
      auto insert_iterator = std::lower_bound(m_events.begin(), m_events.end(),
        event, [] (const auto& lhs, const auto& rhs) {
          return lhs->get_timestamp() < rhs->get_timestamp();
        });
      m_events.insert(insert_iterator, event);
      if(is_active) {
        ++m_active_count;
      }
    }
    if(is_active) {
      m_event_available_condition.notify_one();
    }
  }

  inline void BacktesterEventHandler::add(
      const std::vector<std::shared_ptr<BacktesterEvent>>& events) {
    if(events.empty()) {
      return;
    }
    auto is_active = false;
    {
      auto lock = std::lock_guard(m_mutex);
      for(auto& event : events) {
        is_active |= !event->is_passive();
        auto insert_iterator = std::lower_bound(m_events.begin(),
          m_events.end(), event, [] (const auto& lhs, const auto& rhs) {
            return lhs->get_timestamp() < rhs->get_timestamp();
          });
        if(!event->is_passive()) {
          ++m_active_count;
        }
        m_events.insert(insert_iterator, event);
      }
    }
    if(is_active) {
      m_event_available_condition.notify_one();
    }
  }

  inline void BacktesterEventHandler::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_event_available_condition.notify_one();
    m_event_loop_routine.Wait();
    m_time_environment.Close();
    m_open_state.Close();
    Beam::Routines::FlushPendingRoutines();
  }

  inline void BacktesterEventHandler::event_loop() {
    while(true) {
      auto event = std::shared_ptr<BacktesterEvent>();
      {
        auto lock = std::unique_lock(m_mutex);
        while(m_open_state.IsOpen() && m_active_count == 0) {
          m_event_available_condition.wait(lock);
        }
        if(!m_open_state.IsOpen()) {
          return;
        }
        event = std::move(m_events.front());
        m_events.pop_front();
        if(!event->is_passive()) {
          --m_active_count;
        }
      }
      if(event->get_timestamp() != boost::posix_time::neg_infin) {
        m_time_environment.SetTime(event->get_timestamp());
      }
      event->execute();
      event->complete();
      Beam::Routines::FlushPendingRoutines();
    }
  }
}

#endif
