#ifndef NEXUS_BACKTESTER_TIMER_HPP
#define NEXUS_BACKTESTER_TIMER_HPP
#include <Beam/Queues/QueueWriterPublisher.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Timer.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEvent.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"

namespace Nexus {

  /** Implements a Timer used by the backtester. */
  class BacktesterTimer {
    public:

      /**
       * Constructs a BacktesterTimer.
       * @param interval The time interval before expiring.
       * @param eventHandler The event handler to publish timer events to.
       */
      BacktesterTimer(boost::posix_time::time_duration interval,
        Beam::Ref<BacktesterEventHandler> eventHandler);

      ~BacktesterTimer();

      void Start();

      void Cancel();

      void Wait();

      const Beam::Publisher<Beam::Threading::Timer::Result>&
        GetPublisher() const;

    private:
      friend class TimerBacktesterEvent;
      mutable Beam::Threading::Mutex m_mutex;
      boost::posix_time::time_duration m_interval;
      BacktesterEventHandler* m_eventHandler;
      std::shared_ptr<TimerBacktesterEvent> m_expireEvent;
      std::shared_ptr<TimerBacktesterEvent> m_cancelEvent;
      Beam::QueueWriterPublisher<Beam::Threading::Timer::Result> m_publisher;

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
      TimerBacktesterEvent(BacktesterTimer& timer,
        boost::posix_time::ptime timestamp,
        Beam::Threading::Timer::Result result);

      void Cancel();

      bool IsPassive() const override;

      void Execute() override;

    private:
      mutable Beam::Threading::Mutex m_mutex;
      BacktesterTimer* m_timer;
      Beam::Threading::Timer::Result m_result;
  };

  inline BacktesterTimer::BacktesterTimer(
    boost::posix_time::time_duration interval,
    Beam::Ref<BacktesterEventHandler> eventHandler)
    : m_interval(interval),
      m_eventHandler(eventHandler.Get()) {}

  inline BacktesterTimer::~BacktesterTimer() {
    Cancel();
  }

  inline void BacktesterTimer::Start() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_expireEvent) {
      return;
    }
    m_expireEvent = std::make_shared<TimerBacktesterEvent>(*this,
      m_eventHandler->GetTime() + m_interval,
      Beam::Threading::Timer::Result::EXPIRED);
    m_eventHandler->Add(m_expireEvent);
  }

  inline void BacktesterTimer::Cancel() {
    auto cancelEvent = [&] {
      auto lock = boost::lock_guard(m_mutex);
      if(m_expireEvent && !m_cancelEvent) {
        m_expireEvent->Cancel();
        m_expireEvent = nullptr;
        m_cancelEvent = std::make_shared<TimerBacktesterEvent>(*this,
          boost::posix_time::neg_infin,
          Beam::Threading::Timer::Result::CANCELED);
        m_eventHandler->Add(m_cancelEvent);
      }
      return m_cancelEvent;
    }();
    if(cancelEvent) {
      cancelEvent->Wait();
    }
  }

  inline void BacktesterTimer::Wait() {
    auto event = [&] {
      auto lock = boost::lock_guard(m_mutex);
      if(m_cancelEvent &&
          m_cancelEvent->GetTimestamp() <= m_expireEvent->GetTimestamp()) {
        return m_cancelEvent;
      } else {
        return m_expireEvent;
      }
    }();
    if(event) {
      event->Wait();
    }
  }

  inline const Beam::Publisher<Beam::Threading::Timer::Result>&
      BacktesterTimer::GetPublisher() const {
    return m_publisher;
  }

  inline TimerBacktesterEvent::TimerBacktesterEvent(BacktesterTimer& timer,
    boost::posix_time::ptime timestamp, Beam::Threading::Timer::Result result)
    : BacktesterEvent(timestamp),
      m_timer(&timer),
      m_result(result) {}

  inline void TimerBacktesterEvent::Cancel() {
    auto lock = boost::lock_guard(m_mutex);
    m_result = Beam::Threading::Timer::Result::NONE;
  }

  inline bool TimerBacktesterEvent::IsPassive() const {
    return true;
  }

  inline void TimerBacktesterEvent::Execute() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_result != Beam::Threading::Timer::Result::NONE) {
      auto lock = boost::lock_guard(m_timer->m_mutex);
      m_timer->m_expireEvent = nullptr;
      m_timer->m_cancelEvent = nullptr;
      m_timer->m_publisher.Push(m_result);
    }
  }
}

namespace Beam {
  template<>
  struct ImplementsConcept<Nexus::BacktesterTimer, Threading::Timer> :
    std::true_type {};
}

#endif
