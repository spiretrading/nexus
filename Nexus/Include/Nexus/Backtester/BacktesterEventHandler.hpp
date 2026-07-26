#ifndef NEXUS_BACKTESTER_EVENT_HANDLER_HPP
#define NEXUS_BACKTESTER_EVENT_HANDLER_HPP
#include <algorithm>
#include <deque>
#include <functional>
#include <memory>
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
#include "Nexus/Backtester/ActiveBacktesterEvent.hpp"

namespace Nexus {

  /** Implements an event loop to handle BacktesterEvents. */
  class BacktesterEventHandler {
    public:

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

      /**
       * Sets a task to run after every event's pending Routines are flushed.
       * @param task The task to run, returning <code>true</code> iff it 
       *        performed any work. It is called repeatedly, flushing all
       *        pending Routines after each call, until it returns
       *        <code>false</code>.
       */
      void set_idle_task(std::function<bool ()> task);

      /*** Notifies the event loop to run an idle task. */
      void notify_idle_task();

      /**
       * Suspends the processing of events, blocking until the event being
       * processed.
       */
      void suspend();

      /** Resumes the processing of events. */
      void resume();

      /**
       * Processes the next event while suspended, blocking until it completes.
       * @return The event that was processed, or <code>nullptr</code> if this
       *         event handler is not suspended or has no event to process.
       */
      std::shared_ptr<const BacktesterEvent> advance();

      /** Blocks until the backtester has run to its end time. */
      void wait();

      void close();

    private:
      mutable Beam::Mutex m_mutex;
      boost::posix_time::ptime m_start_time;
      boost::posix_time::ptime m_end_time;
      Beam::Tests::TimeServiceTestEnvironment m_time_environment;
      std::deque<std::shared_ptr<BacktesterEvent>> m_events;
      std::size_t m_active_count;
      bool m_has_processed_active_event;
      bool m_is_suspended;
      bool m_is_dispatching;
      std::size_t m_step_count;
      std::shared_ptr<BacktesterEvent> m_stepped_event;
      std::function<bool ()> m_idle_task;
      bool m_is_idle_task_pending;
      Beam::ConditionVariable m_event_available_condition;
      Beam::ConditionVariable m_active_processed_condition;
      Beam::ConditionVariable m_suspend_condition;
      Beam::RoutineHandler m_event_loop_routine;
      Beam::OpenState m_open_state;

      BacktesterEventHandler(const BacktesterEventHandler&) = delete;
      BacktesterEventHandler& operator =(
        const BacktesterEventHandler&) = delete;
      void event_loop();
  };

  inline BacktesterEventHandler::BacktesterEventHandler(
      boost::posix_time::ptime start, boost::posix_time::ptime end)
      : m_start_time(start),
        m_end_time(end),
        m_active_count(0),
        m_has_processed_active_event(false),
        m_is_suspended(false),
        m_is_dispatching(false),
        m_step_count(0),
        m_is_idle_task_pending(false),
        m_time_environment(m_start_time) {
    try {
      m_event_loop_routine = Beam::spawn(
        std::bind_front(&BacktesterEventHandler::event_loop, this));
    } catch(const std::exception&) {
      close();
      throw;
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
    return m_time_environment.get_time();
  }

  inline void BacktesterEventHandler::add(
      const std::shared_ptr<BacktesterEvent>& event) {
    auto is_active = !event->is_passive();
    {
      auto lock = std::lock_guard(m_mutex);
      auto insert_iterator = std::upper_bound(m_events.begin(), m_events.end(),
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
        auto insert_iterator = std::upper_bound(m_events.begin(),
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

  inline void BacktesterEventHandler::set_idle_task(
      std::function<bool ()> task) {
    auto lock = std::lock_guard(m_mutex);
    m_idle_task = std::move(task);
  }

  inline void BacktesterEventHandler::notify_idle_task() {
    {
      auto lock = std::lock_guard(m_mutex);
      m_is_idle_task_pending = true;
    }
    m_event_available_condition.notify_one();
  }

  inline void BacktesterEventHandler::suspend() {
    auto lock = std::unique_lock(m_mutex);
    m_is_suspended = true;
    while(m_open_state.is_open() && m_is_dispatching) {
      m_suspend_condition.wait(lock);
    }
  }

  inline void BacktesterEventHandler::resume() {
    {
      auto lock = std::lock_guard(m_mutex);
      m_is_suspended = false;
    }
    m_event_available_condition.notify_one();
  }

  inline std::shared_ptr<const BacktesterEvent>
      BacktesterEventHandler::advance() {
    auto lock = std::unique_lock(m_mutex);
    if(!m_is_suspended || m_events.empty()) {
      return nullptr;
    }
    ++m_step_count;
    m_event_available_condition.notify_one();
    while(m_open_state.is_open() && (m_step_count != 0 || m_is_dispatching)) {
      m_suspend_condition.wait(lock);
    }
    return std::move(m_stepped_event);
  }

  inline void BacktesterEventHandler::wait() {
    {
      auto lock = std::unique_lock(m_mutex);
      while(m_open_state.is_open() && !m_has_processed_active_event) {
        m_active_processed_condition.wait(lock);
      }
      if(!m_open_state.is_open()) {
        return;
      }
    }
    auto sentinel = std::make_shared<ActiveBacktesterEvent>(
      m_end_time + boost::posix_time::microseconds(1));
    add(sentinel);
    sentinel->wait();
  }

  inline void BacktesterEventHandler::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    {
      auto lock = std::lock_guard(m_mutex);
      m_event_available_condition.notify_one();
      m_active_processed_condition.notify_all();
      m_suspend_condition.notify_all();
    }
    m_event_loop_routine.wait();
    m_time_environment.close();
    m_open_state.close();
    Beam::flush_pending_routines();
  }

  inline void BacktesterEventHandler::event_loop() {
    while(true) {
      auto event = std::shared_ptr<BacktesterEvent>();
      {
        auto lock = std::unique_lock(m_mutex);
        m_is_dispatching = false;
        if(m_is_suspended) {
          m_suspend_condition.notify_all();
        }
        while(m_open_state.is_open() &&
            (m_active_count == 0 || m_is_suspended) && m_step_count == 0) {
          if(m_is_idle_task_pending && !m_is_suspended) {
            m_is_idle_task_pending = false;
            if(m_idle_task) {
              m_is_dispatching = true;
              auto idle_task = m_idle_task;
              lock.unlock();
              while(idle_task()) {}
              Beam::flush_pending_routines();
              lock.lock();
              m_is_dispatching = false;
              m_suspend_condition.notify_all();
            }
            continue;
          }
          m_event_available_condition.wait(lock);
        }
        if(!m_open_state.is_open()) {
          return;
        }
        m_is_dispatching = true;
        auto is_step = m_step_count != 0;
        if(is_step) {
          --m_step_count;
        }
        event = std::move(m_events.front());
        m_events.pop_front();
        if(!event->is_passive()) {
          --m_active_count;
        }
        if(is_step) {
          m_stepped_event = event;
        }
      }
      if(event->get_timestamp() != boost::posix_time::neg_infin) {
        m_time_environment.set(event->get_timestamp());
      }
      event->execute();
      event->complete();
      if(!event->is_passive()) {
        auto lock = std::lock_guard(m_mutex);
        if(!m_has_processed_active_event) {
          m_has_processed_active_event = true;
          m_active_processed_condition.notify_all();
        }
      }
      Beam::flush_pending_routines();
      auto idle_task = [&] {
        auto lock = std::lock_guard(m_mutex);
        return m_idle_task;
      }();
      if(idle_task) {
        while(idle_task()) {
          Beam::flush_pending_routines();
        }
      }
    }
  }
}

#endif
