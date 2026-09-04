#ifndef NEXUS_BACKTESTER_TIMER_HPP
#define NEXUS_BACKTESTER_TIMER_HPP
#include <memory>
#include <Beam/Queues/QueueWriterPublisher.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/Timer.hpp>
#include "Nexus/Backtester/BacktesterEventHandler.hpp"

namespace Nexus {
  class BacktesterTimer;
  class TimerBacktesterEvent;

namespace Details {
  using BacktesterTimerHandle =
    std::shared_ptr<Beam::Sync<BacktesterTimer*, Beam::Mutex>>;
}

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
        Beam::Ref<BacktesterEventHandler> event_handler);

      ~BacktesterTimer();

      void start();
      void cancel();
      void wait();
      const Beam::Publisher<Result>& get_publisher() const;

    private:
      friend class TimerBacktesterEvent;
      boost::posix_time::time_duration m_interval;
      BacktesterEventHandler* m_event_handler;
      std::shared_ptr<TimerBacktesterEvent> m_expire_event;
      std::shared_ptr<TimerBacktesterEvent> m_cancel_event;
      Beam::QueueWriterPublisher<Result> m_publisher;
      Details::BacktesterTimerHandle m_self;

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

      bool is_passive() const override;
      void execute() override;

    private:
      friend class BacktesterTimer;
      Details::BacktesterTimerHandle m_timer;
      BacktesterTimer::Result m_result;
  };

  inline BacktesterTimer::BacktesterTimer(
      boost::posix_time::time_duration interval,
      Beam::Ref<BacktesterEventHandler> event_handler)
      : m_interval(interval),
        m_event_handler(event_handler.get()) {
    m_self = std::make_shared<Beam::Sync<BacktesterTimer*, Beam::Mutex>>(this);
  }

  inline BacktesterTimer::~BacktesterTimer() {
    m_self->with([&] (auto& timer) {
      timer = nullptr;
      if(m_expire_event) {
        m_expire_event->m_result = Result::NONE;
        m_expire_event = nullptr;
      }
      if(m_cancel_event) {
        m_cancel_event->m_result = Result::NONE;
        m_cancel_event = nullptr;
      }
    });
  }

  inline void BacktesterTimer::start() {
    auto event = m_self->with([&] (const auto& timer) {
      if(m_expire_event) {
        return std::shared_ptr<TimerBacktesterEvent>();
      }
      m_expire_event = std::make_shared<TimerBacktesterEvent>(
        *this, m_event_handler->get_time() + m_interval, Result::EXPIRED);
      return m_expire_event;
    });
    if(event) {
      m_event_handler->add(event);
    }
  }

  inline void BacktesterTimer::cancel() {
    auto is_pending = false;
    auto event = m_self->with([&] (const auto& timer) {
      if(m_expire_event && !m_cancel_event) {
        m_expire_event->m_result = Result::NONE;
        m_expire_event = nullptr;
        m_cancel_event = std::make_shared<TimerBacktesterEvent>(
          *this, boost::posix_time::neg_infin, Result::CANCELED);
        is_pending = true;
      }
      return m_cancel_event;
    });
    if(is_pending) {
      m_event_handler->add(event);
    }
    if(event) {
      event->wait();
    }
  }

  inline void BacktesterTimer::wait() {
    auto event = m_self->with([&] (const auto& timer) {
      if(!m_cancel_event) {
        return m_expire_event;
      }
      if(!m_expire_event ||
          m_cancel_event->get_timestamp() <= m_expire_event->get_timestamp()) {
        return m_cancel_event;
      }
      return m_expire_event;
    });
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
      m_timer(timer.m_self),
      m_result(result) {}

  inline bool TimerBacktesterEvent::is_passive() const {
    return true;
  }

  inline void TimerBacktesterEvent::execute() {
    Beam::with(*m_timer, [&] (const auto& timer) {
      if(!timer || m_result == BacktesterTimer::Result::NONE) {
        return;
      }
      timer->m_expire_event = nullptr;
      timer->m_cancel_event = nullptr;
      timer->m_publisher.push(m_result);
    });
  }
}

#endif
