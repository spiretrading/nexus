#ifndef NEXUS_BACKTESTEREVENT_HPP
#define NEXUS_BACKTESTEREVENT_HPP
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"

namespace Nexus {

  /*! \class BacktesterEvent
      \brief Base class of an event to be handled by the backtester.
   */
  class BacktesterEvent : private boost::noncopyable {
    public:
      virtual ~BacktesterEvent() = default;

      //! Returns the timestamp that this event takes place.
      const boost::posix_time::ptime& GetTimestamp() const;

      //! Waits for this event to be executed.
      void Wait();

      //! Executes this event.
      virtual void Execute() = 0;

    protected:

      //! Constructs a BacktesterEvent.
      /*!
        \param timestamp The time that this event is to take place.
      */
      BacktesterEvent(boost::posix_time::ptime timestamp);

      //! Marks this event as complete/executed.
      void Complete();

    private:
      mutable Beam::Threading::Mutex m_mutex;
      bool m_isComplete;
      Beam::Threading::ConditionVariable m_isCompleteCondition;
      boost::posix_time::ptime m_timestamp;
  };

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
