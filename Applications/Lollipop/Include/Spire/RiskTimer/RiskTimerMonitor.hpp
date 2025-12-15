#ifndef SPIRE_RISK_TIMER_MONITOR_HPP
#define SPIRE_RISK_TIMER_MONITOR_HPP
#include <Beam/Pointers/Ref.hpp>
#include "Nexus/RiskService/RiskState.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/RiskTimer/RiskTimer.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /**
   * Monitors the user's RiskState and shows/hides the user's shutdown timer
   * when the RiskState makes a transition.
   */
  class RiskTimerMonitor {
    public:

      /**
       * Constructs a RiskTimerMonitor.
       * @param userProfile The user's profile.
       */
      explicit RiskTimerMonitor(Beam::Ref<UserProfile> userProfile);

      ~RiskTimerMonitor();

      /** Loads the data needed to monitor the user's RiskState. */
      void Load();

    private:
      UserProfile* m_userProfile;
      std::shared_ptr<RiskTimerModel> m_model;
      std::unique_ptr<RiskTimerDialog> m_dialog;
      EventHandler m_eventHandler;

      RiskTimerMonitor(const RiskTimerMonitor&) = delete;
      RiskTimerMonitor& operator =(const RiskTimerMonitor&) = delete;
      void OnRiskState(const Nexus::RiskState& riskState);
  };
}

#endif
