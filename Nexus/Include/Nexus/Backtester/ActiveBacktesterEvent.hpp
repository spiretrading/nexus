#ifndef NEXUS_ACTIVE_BACKTESTER_EVENT_HPP
#define NEXUS_ACTIVE_BACKTESTER_EVENT_HPP
#include "Nexus/Backtester/BacktesterEvent.hpp"

namespace Nexus {

  /** An active event that can be used to advance time in the backtester. */
  class ActiveBacktesterEvent : public BacktesterEvent {
    public:

      /** Constructs an ActiveBacktesterEvent with an immediate timestamp. */
      ActiveBacktesterEvent();

      /**
       * Constructs an ActiveBacktesterEvent.
       * @param timestamp The time that this event is to take place.
       */
      ActiveBacktesterEvent(boost::posix_time::ptime timestamp);

      bool IsPassive() const override;

      void Execute() override;
  };

  inline ActiveBacktesterEvent::ActiveBacktesterEvent()
    : ActiveBacktesterEvent(boost::posix_time::neg_infin) {}

  inline ActiveBacktesterEvent::ActiveBacktesterEvent(
    boost::posix_time::ptime timestamp)
    : BacktesterEvent(timestamp) {}

  inline bool ActiveBacktesterEvent::IsPassive() const {
    return false;
  }

  inline void ActiveBacktesterEvent::Execute() {}
}

#endif
