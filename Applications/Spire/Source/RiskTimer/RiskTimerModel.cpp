#include "Spire/RiskTimer/RiskTimerModel.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

namespace {
  const unsigned int UPDATE_INTERVAL = 100;
}

RiskTimerModel::RiskTimerModel(Ref<UserProfile> userProfile)
    : m_userProfile(userProfile.Get()),
      m_timeRemaining(seconds(0)),
      m_timeRemainingTimer(milliseconds(UPDATE_INTERVAL)) {
  connect(&m_updateTimer, &QTimer::timeout, this,
    &RiskTimerModel::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
  std::function<void (const Timer::Result&)> expiredSlot =
    std::bind(&RiskTimerModel::OnTimeRemainingExpired, this,
    std::placeholders::_1);
  m_timeRemainingTimer.GetPublisher().Monitor(
    m_slotHandler.GetSlot(expiredSlot));
  m_timeRemainingTimer.Start();
}

const time_duration& RiskTimerModel::GetTimeRemaining() const {
  return m_timeRemaining;
}

void RiskTimerModel::SetTimeRemaining(const time_duration& timeRemaining) {
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
    ptime currentTime =
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

void RiskTimerModel::OnUpdateTimer() {
  HandleTasks(m_slotHandler);
}
