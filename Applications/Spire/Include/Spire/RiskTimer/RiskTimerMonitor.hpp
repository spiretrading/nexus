#ifndef SPIRE_RISKTIMERMONITOR_HPP
#define SPIRE_RISKTIMERMONITOR_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <boost/noncopyable.hpp>
#include <QObject>
#include <QTimer>
#include "Nexus/RiskService/RiskState.hpp"
#include "Spire/RiskTimer/RiskTimer.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class RiskTimerMonitor
      \brief Monitors the user's RiskState and shows/hides the user's shutdown
             timer when the RiskState makes a transition.
   */
  class RiskTimerMonitor : public QObject, private boost::noncopyable {
    public:

      //! Constructs a RiskTimerMonitor.
      /*!
        \param userProfile The user's profile.
      */
      RiskTimerMonitor(Beam::RefType<UserProfile> userProfile);

      ~RiskTimerMonitor();

      //! Loads the data needed to monitor the user's RiskState.
      void Load();

    private:
      QTimer m_updateTimer;
      UserProfile* m_userProfile;
      std::shared_ptr<RiskTimerModel> m_model;
      std::unique_ptr<RiskTimerDialog> m_dialog;
      Beam::TaskQueue m_slotHandler;

      void OnRiskState(const Nexus::RiskService::RiskState& riskState);
      void OnUpdateTimer();
  };
}

#endif
