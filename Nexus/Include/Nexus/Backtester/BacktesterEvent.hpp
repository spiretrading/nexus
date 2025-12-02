#ifndef NEXUS_BACKTESTER_EVENT_HPP
#define NEXUS_BACKTESTER_EVENT_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>

namespace Nexus {

  /** Base class of an event to be handled by the backtester. */
  class BacktesterEvent {
    public:
      virtual ~BacktesterEvent() = default;

      /** Returns the timestamp that this event takes place. */
      boost::posix_time::ptime get_timestamp() const;

      /** Waits for this event to be executed. */
      void wait();

      /** Whether this event should trigger an update. */
      virtual bool is_passive() const;

      /** Executes this event. */
      virtual void execute() = 0;

    protected:

      /**
       * Constructs a BacktesterEvent.
       * @param timestamp The time that this event is to take place.
       */
      explicit BacktesterEvent(boost::posix_time::ptime timestamp) noexcept;

      /** Marks this event as complete/executed. */
      void complete();

    private:
      friend class BacktesterEventHandler;
      mutable Beam::Mutex m_mutex;
      bool m_is_complete;
      Beam::ConditionVariable m_is_complete_condition;
      boost::posix_time::ptime m_timestamp;

      BacktesterEvent(const BacktesterEvent&) = delete;
      BacktesterEvent& operator =(const BacktesterEvent&) = delete;
  };

  inline boost::posix_time::ptime BacktesterEvent::get_timestamp() const {
    return m_timestamp;
  }

  inline void BacktesterEvent::wait() {
    auto lock = std::unique_lock(m_mutex);
    while(!m_is_complete) {
      m_is_complete_condition.wait(lock);
    }
  }

  inline bool BacktesterEvent::is_passive() const {
    return false;
  }

  inline BacktesterEvent::BacktesterEvent(
    boost::posix_time::ptime timestamp) noexcept
    : m_is_complete(false),
      m_timestamp(timestamp) {}

  inline void BacktesterEvent::complete() {
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_is_complete) {
        return;
      }
      m_is_complete = true;
    }
    m_is_complete_condition.notify_all();
  }
}

#endif
