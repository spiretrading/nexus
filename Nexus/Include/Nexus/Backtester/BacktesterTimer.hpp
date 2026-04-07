#ifndef NEXUS_BACKTESTER_TIMER_HPP
#define NEXUS_BACKTESTER_TIMER_HPP
#include <Beam/Queues/QueueWriterPublisher.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/TimeService/Timer.hpp>
#include "Nexus/Backtester/BacktesterEventHandler.hpp"

namespace Nexus {
  class TimerBacktesterEvent;

  /** Implements a Timer used by the backtester. */
  class BacktesterTimer {
    public:
      using Result = Beam::Timer::Result;

      /**
       * Constructs a BacktesterTimer.
       * @param interval The time interval before expiring.
       * @param event_handler The event handler to publish timer events to.
       */
      BacktesterTimer(boost::posix_time::time_duration interval,
        Beam::Ref<BacktesterEventHandler> event_handler) noexcept;

      ~BacktesterTimer();

      void start();
      void cancel();
      void wait();
      const Beam::Publisher<Result>& get_publisher() const;

    private:
      friend class TimerBacktesterEvent;
      mutable Beam::Mutex m_mutex;
      boost::posix_time::time_duration m_interval;
      BacktesterEventHandler* m_event_handler;
      std::shared_ptr<TimerBacktesterEvent> m_expire_event;
      std::shared_ptr<TimerBacktesterEvent> m_cancel_event;
      Beam::QueueWriterPublisher<Result> m_publisher;

      BacktesterTimer(const BacktesterTimer&) = delete;
      BacktesterTimer& operator =(const BacktesterTimer&) = delete;
  };

  /** Represents a Timer event. */
  class TimerBacktesterEvent : public BacktesterEvent {
    public:

      /**
       * Constructs a TimerBacktesterEvent.
       * @param timer The BacktesterTimer that produced the event.
       * @param timestamp The time this event is to be executed.
       * @param result The Timer result.
       */
      TimerBacktesterEvent(
        BacktesterTimer& timer, boost::posix_time::ptime timestamp,
        BacktesterTimer::Result result) noexcept;

      void cancel();
      bool is_passive() const override;
      void execute() override;

    private:
      mutable Beam::Mutex m_mutex;
      BacktesterTimer* m_timer;
      BacktesterTimer::Result m_result;
  };

  inline BacktesterTimer::BacktesterTimer(
    boost::posix_time::time_duration interval,
    Beam::Ref<BacktesterEventHandler> event_handler) noexcept
    : m_interval(interval),
      m_event_handler(event_handler.get()) {}

  inline BacktesterTimer::~BacktesterTimer() {
    cancel();
  }

  inline void BacktesterTimer::start() {
    auto lock = std::lock_guard(m_mutex);
    if(m_expire_event) {
      return;
    }
    m_expire_event = std::make_shared<TimerBacktesterEvent>(
      *this, m_event_handler->get_time() + m_interval, Result::EXPIRED);
    m_event_handler->add(m_expire_event);
  }

  inline void BacktesterTimer::cancel() {
    auto cancel_event = [&] {
      auto lock = std::lock_guard(m_mutex);
      if(m_expire_event && !m_cancel_event) {
        m_expire_event->cancel();
        m_expire_event = nullptr;
        m_cancel_event = std::make_shared<TimerBacktesterEvent>(
          *this, boost::posix_time::neg_infin, Result::CANCELED);
        m_event_handler->add(m_cancel_event);
      }
      return m_cancel_event;
    }();
    if(cancel_event) {
      cancel_event->wait();
    }
  }

  inline void BacktesterTimer::wait() {
    auto event = [&] {
      auto lock = std::lock_guard(m_mutex);
      if(m_cancel_event &&
          m_cancel_event->get_timestamp() <= m_expire_event->get_timestamp()) {
        return m_cancel_event;
      } else {
        return m_expire_event;
      }
    }();
    if(event) {
      event->wait();
    }
  }

  inline const Beam::Publisher<BacktesterTimer::Result>&
      BacktesterTimer::get_publisher() const {
    return m_publisher;
  }

  inline TimerBacktesterEvent::TimerBacktesterEvent(BacktesterTimer& timer,
    boost::posix_time::ptime timestamp, BacktesterTimer::Result result) noexcept
    : BacktesterEvent(timestamp),
      m_timer(&timer),
      m_result(result) {}

  inline void TimerBacktesterEvent::cancel() {
    auto lock = std::lock_guard(m_mutex);
    m_result = BacktesterTimer::Result::NONE;
  }

  inline bool TimerBacktesterEvent::is_passive() const {
    return true;
  }

  inline void TimerBacktesterEvent::execute() {
    auto lock = std::lock_guard(m_mutex);
    if(m_result != BacktesterTimer::Result::NONE) {
      auto lock = std::lock_guard(m_timer->m_mutex);
      m_timer->m_expire_event = nullptr;
      m_timer->m_cancel_event = nullptr;
      m_timer->m_publisher.push(m_result);
    }
  }
}

#endif
