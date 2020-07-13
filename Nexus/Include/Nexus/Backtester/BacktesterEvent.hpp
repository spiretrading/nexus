#ifndef NEXUS_BACKTESTER_EVENT_HPP
#define NEXUS_BACKTESTER_EVENT_HPP
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"

namespace Nexus {

  /** Base class of an event to be handled by the backtester. */
  class BacktesterEvent : private boost::noncopyable {
    public:
      virtual ~BacktesterEvent() = default;

      /** Returns the timestamp that this event takes place. */
      boost::posix_time::ptime GetTimestamp() const;

      /** Waits for this event to be executed. */
      void Wait();

      /** Executes this event. */
      virtual void Execute() = 0;

    protected:

      /**
       * Constructs a BacktesterEvent.
       * @param timestamp The time that this event is to take place.
       */
      BacktesterEvent(boost::posix_time::ptime timestamp);

      /** Marks this event as complete/executed. */
      void Complete();

    private:
      mutable Beam::Threading::Mutex m_mutex;
      bool m_isComplete;
      Beam::Threading::ConditionVariable m_isCompleteCondition;
      boost::posix_time::ptime m_timestamp;
  };

  inline boost::posix_time::ptime BacktesterEvent::GetTimestamp() const {
    return m_timestamp;
  }

  inline void BacktesterEvent::Wait() {
    auto lock = boost::unique_lock(m_mutex);
    while(!m_isComplete) {
      m_isCompleteCondition.wait(lock);
    }
  }

  inline BacktesterEvent::BacktesterEvent(boost::posix_time::ptime timestamp)
    : m_isComplete(false),
      m_timestamp(timestamp) {}

  inline void BacktesterEvent::Complete() {
    {
      auto lock = boost::lock_guard(m_mutex);
      if(m_isComplete) {
        return;
      }
      m_isComplete = true;
    }
    m_isCompleteCondition.notify_one();
  }
}

#endif
