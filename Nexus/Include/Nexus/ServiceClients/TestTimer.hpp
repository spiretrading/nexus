#ifndef NEXUS_TESTTIMER_HPP
#define NEXUS_TESTTIMER_HPP
#include <boost/noncopyable.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/ServiceClients/TestEnvironment.hpp"

namespace Nexus {

  /*! \class TestTimer
      \brief The type of Timer used by the TestEnvironment.
   */
  class TestTimer : private boost::noncopyable {
    public:

      //! Constructs a TestTimer.
      /*!
        \param interval The time interval before expiring.
        \param environment The TestEnvironment this Timer belongs to.
      */
      TestTimer(boost::posix_time::time_duration interval,
        Beam::RefType<TestEnvironment> environment);

      ~TestTimer();

      void Start();

      void Cancel();

      void Wait();

      const Beam::Publisher<Beam::Threading::Timer::Result>&
        GetPublisher() const;

    private:
      friend class TestEnvironment;
      friend void Trigger(TestTimer& timer);
      mutable boost::mutex m_mutex;
      boost::posix_time::time_duration m_interval;
      TestEnvironment* m_environment;
      bool m_hasStarted;
      Beam::Threading::TriggerTimer m_timer;
  };

  inline TestTimer::TestTimer(boost::posix_time::time_duration interval,
      Beam::RefType<TestEnvironment> environment)
      : m_interval{std::move(interval)},
        m_environment{environment.Get()},
        m_hasStarted{false} {}

  inline TestTimer::~TestTimer() {
    Cancel();
  }

  inline void TestTimer::Start() {
    {
      boost::lock_guard<boost::mutex> lock{m_mutex};
      if(m_hasStarted) {
        return;
      }
      m_hasStarted = true;
    }
    m_timer.Start();
    m_environment->Add(this);
  }

  inline void TestTimer::Cancel() {
    {
      boost::lock_guard<boost::mutex> lock{m_mutex};
      if(!m_hasStarted) {
        return;
      }
      m_hasStarted = false;
    }
    m_environment->Remove(this);
    m_timer.Cancel();
  }

  inline void TestTimer::Wait() {
    m_timer.Wait();
  }

  inline const Beam::Publisher<Beam::Threading::Timer::Result>&
      TestTimer::GetPublisher() const {
    return m_timer.GetPublisher();
  }

  inline void TestEnvironment::Add(TestTimer* timer) {
    if(timer->m_interval <= boost::posix_time::seconds(0)) {
      timer->m_timer.Trigger();
      return;
    }
    TimerEntry entry{timer, timer->m_interval};
    boost::lock_guard<Beam::Threading::Mutex> lock{m_timeMutex};
    m_timers.PushBack(entry);
    m_nextTrigger = std::min(m_nextTrigger, timer->m_interval);
  }

  inline void Trigger(TestTimer& timer) {
    {
      boost::lock_guard<boost::mutex> lock{timer.m_mutex};
      timer.m_hasStarted = false;
    }
    timer.m_timer.Trigger();
  }
}

namespace Beam {
  template<>
  struct ImplementsConcept<Nexus::TestTimer, Threading::Timer> :
    std::true_type {};
}

#endif
