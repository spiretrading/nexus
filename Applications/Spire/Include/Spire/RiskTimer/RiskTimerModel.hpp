#ifndef SPIRE_RISK_TIMER_MODEL_HPP
#define SPIRE_RISK_TIMER_MODEL_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/signals2/signal.hpp>
#include <QObject>
#include "Spire/Async/EventHandler.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/RiskTimer/RiskTimer.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Models the countdown timer used to transition among RiskStates. */
  class RiskTimerModel : public QObject {
    public:

      /**
       * Signals an update to the time remaining before a transition.
       * @param timeRemaining The time remaining before the next transition.
       */
      using TimeRemainingSignal = boost::signals2::signal<
        void (boost::posix_time::time_duration timeRemaining)>;

      /**
       * Constructs a RiskTimerModel.
       * @param userProfile The user's profile.
       */
      explicit RiskTimerModel(Beam::Ref<UserProfile> userProfile);

      /** Returns the time remaining before the next RiskState transition. */
      boost::posix_time::time_duration GetTimeRemaining() const;

      /** Sets the time remaining before the next RiskState transition. */
      void SetTimeRemaining(boost::posix_time::time_duration timeRemaining);

      /**
       * Connects a slot to the TimeRemainingSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectTimeRemainingSignal(
        const TimeRemainingSignal::slot_type& slot) const;

    private:
      UserProfile* m_userProfile;
      boost::posix_time::time_duration m_timeRemaining;
      Beam::LiveTimer m_timeRemainingTimer;
      boost::signals2::scoped_connection m_timeRemainingConnection;
      boost::posix_time::ptime m_lastTimeCheck;
      mutable TimeRemainingSignal m_timeRemainingSignal;
      EventHandler m_eventHandler;

      void OnTimeRemainingExpired(const Beam::Timer::Result& result);
  };
}

#endif
