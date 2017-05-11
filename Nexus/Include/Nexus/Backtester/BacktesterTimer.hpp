#ifndef NEXUS_BACKTESTERTIMER_HPP
#define NEXUS_BACKTESTERTIMER_HPP
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include <boost/noncopyable.hpp>
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
      BacktesterEnvironment* m_environment;
      Beam::TimeService::Tests::TestTimer m_testTimer;
      Beam::MultiQueueWriter<Beam::Threading::Timer::Result> m_publisher;
      Beam::RoutineTaskQueue m_tasks;

      void OnExpired(Beam::Threading::Timer::Result result);
  };

  class TimerBacktesterEvent : public BacktesterEvent {
    public:
      TimerBacktesterEvent(BacktesterTimer& timer,
        Beam::Threading::Timer::Result result,
        boost::posix_time::ptime timestamp);

      virtual ~TimerBacktesterEvent() = default;

      BacktesterTimer& GetTimer();

      Beam::Threading::Timer::Result GetResult() const;

      virtual void Execute() override;

    private:
      BacktesterTimer* m_timer;
      Beam::Threading::Timer::Result m_result;
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
    auto event = std::make_shared<TimerBacktesterEvent>(*this,
      result, m_environment->m_testEnvironment.GetTimeEnvironment().GetTime());
    m_environment->Push(event);
    event->Wait();
  }

  inline TimerBacktesterEvent::TimerBacktesterEvent(BacktesterTimer& timer,
      Beam::Threading::Timer::Result result,
      boost::posix_time::ptime timestamp)
      : BacktesterEvent{timestamp},
        m_timer{&timer},
        m_result{result} {}

  inline BacktesterTimer& TimerBacktesterEvent::GetTimer() {
    return *m_timer;
  }

  inline Beam::Threading::Timer::Result
      TimerBacktesterEvent::GetResult() const {
    return m_result;
  }

  inline void TimerBacktesterEvent::Execute() {
    m_timer->m_publisher.Push(m_result);
    Beam::Routines::FlushPendingRoutines();
    Complete();
  }
}

namespace Beam {
  template<>
  struct ImplementsConcept<Nexus::BacktesterTimer, Threading::Timer> :
    std::true_type {};
}

#endif
