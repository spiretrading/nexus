#ifndef SPIRE_RISKTIMERMODEL_HPP
#define SPIRE_RISKTIMERMODEL_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/signals2/signal.hpp>
#include <QTimer>
#include "Spire/RiskTimer/RiskTimer.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class RiskTimerModel
      \brief Models the countdown timer used to transition among RiskStates.
   */
  class RiskTimerModel : public QObject {
    public:

      //! Signals an update to the time remaining before a transition.
      /*!
        \param timeRemaining The time remaining before the next transition.
      */
      typedef boost::signals2::signal<
        void (const boost::posix_time::time_duration& timeRemaining)>
        TimeRemainingSignal;

      //! Constructs a RiskTimerModel.
      /*!
        \param userProfile The user's profile.
      */
      RiskTimerModel(Beam::Ref<UserProfile> userProfile);

      //! Returns the time remaining before the next RiskState transition.
      const boost::posix_time::time_duration& GetTimeRemaining() const;

      //! Sets the time remaining before the next RiskState transition.
      void SetTimeRemaining(
        const boost::posix_time::time_duration& timeRemaining);

      //! Connects a slot to the TimeRemainingSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectTimeRemainingSignal(
        const TimeRemainingSignal::slot_type& slot) const;

    private:
      Beam::TaskQueue m_slotHandler;
      QTimer m_updateTimer;
      UserProfile* m_userProfile;
      boost::posix_time::time_duration m_timeRemaining;
      Beam::Threading::LiveTimer m_timeRemainingTimer;
      boost::signals2::scoped_connection m_timeRemainingConnection;
      boost::posix_time::ptime m_lastTimeCheck;
      mutable TimeRemainingSignal m_timeRemainingSignal;

      void OnTimeRemainingExpired(const Beam::Threading::Timer::Result& result);
      void OnUpdateTimer();
  };
}

#endif
