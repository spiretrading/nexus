#ifndef NEXUS_BACKTESTERTIMER_HPP
#define NEXUS_BACKTESTERTIMER_HPP
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEnvironment.hpp"

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
      friend void Details::TriggerTimer(TimerExpiredEvent& event);
      BacktesterEnvironment* m_environment;
      Beam::TimeService::Tests::TestTimer m_testTimer;
      Beam::MultiQueueWriter<Beam::Threading::Timer::Result> m_publisher;
      Beam::RoutineTaskQueue m_tasks;

      void OnExpired(Beam::Threading::Timer::Result result);
  };

  inline BacktesterTimer::BacktesterTimer(
      boost::posix_time::time_duration interval,
      Beam::RefType<BacktesterEnvironment> environment)
      : m_environment{environment.Get()},
        m_testTimer{interval,
          Beam::Ref(environment->m_testEnvironment.GetTimeEnvironment())} {
    m_testTimer.GetPublisher().Monitor(
      m_tasks.GetSlot<Beam::Threading::Timer::Result>(
      std::bind(&BacktesterTimer::OnExpired, this, std::placeholders::_1)));
  }

  inline BacktesterTimer::~BacktesterTimer() {
    Cancel();
  }

  inline void BacktesterTimer::Start() {
    m_testTimer.Start();
  }

  inline void BacktesterTimer::Cancel() {
    m_testTimer.Cancel();
  }

  inline void BacktesterTimer::Wait() {}

  inline const Beam::Publisher<Beam::Threading::Timer::Result>&
      BacktesterTimer::GetPublisher() const {
    return m_publisher;
  }

  inline void BacktesterTimer::OnExpired(
      Beam::Threading::Timer::Result result) {
    Details::TimerExpiredEvent event{*this, result,
      m_environment->m_testEnvironment.GetTimeEnvironment().GetTime()};
    m_environment->Expire(event);
  }

namespace Details {
  inline void TriggerTimer(TimerExpiredEvent& event) {
    event.m_timer->m_publisher.Push(event.m_result);
    Beam::Routines::FlushPendingRoutines();
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{event.m_mutex};
      event.m_isTriggered = true;
    }
    event.m_expiredCondition.notify_one();
  }
}
}

namespace Beam {
  template<>
  struct ImplementsConcept<Nexus::BacktesterTimer, Threading::Timer> :
    std::true_type {};
}

#endif
