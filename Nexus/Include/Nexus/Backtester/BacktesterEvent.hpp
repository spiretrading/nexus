#ifndef NEXUS_BACKTESTEREVENT_HPP
#define NEXUS_BACKTESTEREVENT_HPP
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"

namespace Nexus {
  class BacktesterEvent : private boost::noncopyable {
    public:
      BacktesterEvent();

      virtual ~BacktesterEvent() = default;

      const boost::posix_time::ptime& GetTimestamp() const;

      void Wait();

      virtual void Execute() = 0;

    protected:
      BacktesterEvent(boost::posix_time::ptime timestamp);

      void Complete();

    private:
      mutable Beam::Threading::Mutex m_mutex;
      bool m_isComplete;
      Beam::Threading::ConditionVariable m_isCompleteCondition;
      boost::posix_time::ptime m_timestamp;
  };

  inline BacktesterEvent::BacktesterEvent()
      : m_timestamp{boost::posix_time::pos_infin} {}

  inline const boost::posix_time::ptime&
      BacktesterEvent::GetTimestamp() const {
    return m_timestamp;
  }

  inline void BacktesterEvent::Wait() {
    boost::unique_lock<Beam::Threading::Mutex> lock{m_mutex};
    while(!m_isComplete) {
      m_isCompleteCondition.wait(lock);
    }
  }

  inline BacktesterEvent::BacktesterEvent(boost::posix_time::ptime timestamp)
      : m_isComplete{false},
        m_timestamp{timestamp} {}

  inline void BacktesterEvent::Complete() {
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
      if(m_isComplete) {
        return;
      }
      m_isComplete = true;
    }
    m_isCompleteCondition.notify_one();
  }
}

#endif
