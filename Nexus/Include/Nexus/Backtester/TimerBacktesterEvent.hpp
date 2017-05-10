#ifndef NEXUS_TIMERBACKTESTEREVENT_HPP
#define NEXUS_TIMERBACKTESTEREVENT_HPP
#include <Beam/Threading/Timer.hpp>
#include "Nexus/Backtester/BacktesterEvent.hpp"
#include "Nexus/Backtester/Backtester.hpp"

namespace Nexus {
  class TimerBacktesterEvent : public BacktesterEvent {
    public:
      TimerBacktesterEvent(BacktesterTimer& timer,
        Beam::Threading::Timer::Result result,
        boost::posix_time::ptime timestamp);

      virtual ~TimerBacktesterEvent() = default;

      BacktesterTimer& GetTimer();

      Beam::Threading::Timer::Result GetResult() const;

    private:
      BacktesterTimer* m_timer;
      Beam::Threading::Timer::Result m_result;
  };

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
}

#endif
