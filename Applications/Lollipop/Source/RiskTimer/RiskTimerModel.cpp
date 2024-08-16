#include "Spire/RiskTimer/RiskTimerModel.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;

RiskTimerModel::RiskTimerModel(Ref<UserProfile> userProfile)
    : m_userProfile(userProfile.Get()),
      m_timeRemaining(seconds(0)),
      m_timeRemainingTimer(seconds(1)) {
  m_timeRemainingTimer.GetPublisher().Monitor(
    m_eventHandler.get_slot<Timer::Result>(
      std::bind_front(&RiskTimerModel::OnTimeRemainingExpired, this)));
  m_timeRemainingTimer.Start();
}

time_duration RiskTimerModel::GetTimeRemaining() const {
  return m_timeRemaining;
}

void RiskTimerModel::SetTimeRemaining(time_duration timeRemaining) {
  m_timeRemaining = timeRemaining;
  m_lastTimeCheck =
    m_userProfile->GetServiceClients().GetTimeClient().GetTime();
}

connection RiskTimerModel::ConnectTimeRemainingSignal(
    const TimeRemainingSignal::slot_type& slot) const {
  return m_timeRemainingSignal.connect(slot);
}

void RiskTimerModel::OnTimeRemainingExpired(const Timer::Result& result) {
  if(m_timeRemaining != seconds(0)) {
    auto currentTime =
      m_userProfile->GetServiceClients().GetTimeClient().GetTime();
    m_timeRemaining -= (currentTime - m_lastTimeCheck);
    m_lastTimeCheck = currentTime;
    if(m_timeRemaining < seconds(0)) {
      m_timeRemaining = seconds(0);
    }
    m_timeRemainingSignal(m_timeRemaining);
  }
  m_timeRemainingTimer.Start();
}
