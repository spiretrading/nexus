#ifndef NEXUS_BACKTESTERTIMER_HPP
#define NEXUS_BACKTESTERTIMER_HPP
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterEvent.hpp"

namespace Nexus {

  /*! \class BacktesterTimer
      \brief Implements a Timer used by the backtester.
   */
  class BacktesterTimer : private boost::noncopyable {
    public:

      //! Constructs a BacktesterTimer.
      /*!
        \param interval The time interval before expiring.
        \param environment The TestEnvironment this Timer belongs to.
      */
      BacktesterTimer(boost::posix_time::time_duration interval,
        Beam::RefType<BacktesterEnvironment> environment);

      ~BacktesterTimer();

      void Start();

      void Cancel();

      void Wait();

      const Beam::Publisher<Beam::Threading::Timer::Result>&
        GetPublisher() const;

    private:
      friend class TimerBacktesterEvent;
      mutable std::shared_ptr<boost::mutex> m_mutex;
      boost::posix_time::time_duration m_interval;
      BacktesterEnvironment* m_environment;
      std::shared_ptr<bool> m_isAlive;
      int m_iteration;
      std::shared_ptr<TimerBacktesterEvent> m_expireEvent;
      std::shared_ptr<TimerBacktesterEvent> m_cancelEvent;
      Beam::MultiQueueWriter<Beam::Threading::Timer::Result> m_publisher;
  };

  class TimerBacktesterEvent : public BacktesterEvent {
    public:
      TimerBacktesterEvent(BacktesterTimer& timer,
        boost::posix_time::ptime timestamp,
        Beam::Threading::Timer::Result result);

      virtual void Execute() override;

    private:
      mutable std::shared_ptr<boost::mutex> m_mutex;
      BacktesterTimer* m_timer;
      std::shared_ptr<bool> m_isAlive;
      Beam::Threading::Timer::Result m_result;
      int m_iteration;
  };

  inline BacktesterTimer::BacktesterTimer(
      boost::posix_time::time_duration interval,
      Beam::RefType<BacktesterEnvironment> environment)
      : m_mutex{std::make_shared<boost::mutex>()},
        m_interval{interval},
        m_environment{environment.Get()},
        m_isAlive{std::make_shared<bool>(true)},
        m_iteration{0} {}

  inline BacktesterTimer::~BacktesterTimer() {
    Cancel();
    boost::lock_guard<boost::mutex> lock{*m_mutex};
    *m_isAlive = false;
  }

  inline void BacktesterTimer::Start() {
    {
      boost::lock_guard<boost::mutex> lock{*m_mutex};
      if(m_expireEvent != nullptr) {
        return;
      }
      m_expireEvent = std::make_shared<TimerBacktesterEvent>(*this,
        m_environment->GetTestEnvironment().GetTimeEnvironment().GetTime() +
        m_interval, Beam::Threading::Timer::Result::EXPIRED);
    }
    m_environment->Add(m_expireEvent);
  }

  inline void BacktesterTimer::Cancel() {
    auto addEvent = false;
    std::shared_ptr<TimerBacktesterEvent> cancelEvent;
    {
      boost::lock_guard<boost::mutex> lock{*m_mutex};
      if(m_expireEvent == nullptr) {
        return;
      } else if(m_cancelEvent == nullptr) {
        m_cancelEvent = std::make_shared<TimerBacktesterEvent>(*this,
          m_environment->GetTestEnvironment().GetTimeEnvironment().GetTime(),
          Beam::Threading::Timer::Result::CANCELED);
        addEvent = true;
      }
      cancelEvent = m_cancelEvent;
    }
    if(addEvent) {
      m_environment->Add(cancelEvent);
    }
    cancelEvent->Wait();
  }

  inline void BacktesterTimer::Wait() {
    std::shared_ptr<TimerBacktesterEvent> event;
    {
      boost::lock_guard<boost::mutex> lock{*m_mutex};
      if(m_expireEvent == nullptr) {
        return;
      }
      if(m_cancelEvent != nullptr &&
          m_cancelEvent->GetTimestamp() < m_expireEvent->GetTimestamp()) {
        event = m_cancelEvent;
      } else {
        event = m_expireEvent;
      }
    }
    event->Wait();
  }

  inline const Beam::Publisher<Beam::Threading::Timer::Result>&
      BacktesterTimer::GetPublisher() const {
    return m_publisher;
  }

  inline TimerBacktesterEvent::TimerBacktesterEvent(BacktesterTimer& timer,
      boost::posix_time::ptime timestamp,
      Beam::Threading::Timer::Result result)
      : BacktesterEvent{timestamp},
        m_mutex{timer.m_mutex},
        m_timer{&timer},
        m_isAlive{m_timer->m_isAlive},
        m_result{result},
        m_iteration{timer.m_iteration} {}

  inline void TimerBacktesterEvent::Execute() {
    {
      boost::lock_guard<boost::mutex> lock{*m_mutex};
      if(!*m_isAlive || m_timer->m_iteration != m_iteration) {
        return;
      }
      m_timer->m_publisher.Push(m_result);
      m_timer->m_expireEvent = nullptr;
      m_timer->m_cancelEvent = nullptr;
      ++m_timer->m_iteration;
    }
    Complete();
  }
}

namespace Beam {
  template<>
  struct ImplementsConcept<Nexus::BacktesterTimer, Threading::Timer> :
    std::true_type {};
}

#endif
