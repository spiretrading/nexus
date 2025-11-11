#include "Spire/RiskTimer/RiskTimerMonitor.hpp"
#include "Spire/RiskTimer/RiskTimerModel.hpp"
#include "Spire/RiskTimer/RiskTimerDialog.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

RiskTimerMonitor::RiskTimerMonitor(Ref<UserProfile> userProfile)
  : m_userProfile(userProfile.get()) {}

RiskTimerMonitor::~RiskTimerMonitor() {
  if(m_dialog) {
    m_userProfile->GetRiskTimerProperties().SetRiskTimerDialogInitialPosition(
      m_dialog->pos());
  }
}

void RiskTimerMonitor::Load() {
  auto account =
    m_userProfile->GetClients().get_service_locator_client().get_account();
  m_userProfile->GetClients().get_administration_client().
    get_risk_state_publisher(account).monitor(
      m_eventHandler.get_slot<RiskState>(
        std::bind_front(&RiskTimerMonitor::OnRiskState, this)));
}

void RiskTimerMonitor::OnRiskState(const RiskState& riskState) {
  if(riskState.m_type == RiskState::Type::ACTIVE) {
    if(m_dialog) {
      m_userProfile->GetRiskTimerProperties().SetRiskTimerDialogInitialPosition(
        m_dialog->pos());
      m_dialog->close();
      m_dialog = nullptr;
      m_model = nullptr;
    }
  } else if(riskState.m_type == RiskState::Type::CLOSE_ORDERS ||
      riskState.m_type == RiskState::Type::DISABLED) {
    if(!m_dialog) {
      m_model = std::make_shared<RiskTimerModel>(Ref(*m_userProfile));
      if(riskState.m_expiry == pos_infin) {
        m_model->SetTimeRemaining(seconds(0));
      } else {
        auto currentTime =
          m_userProfile->GetClients().get_time_client().get_time();
        m_model->SetTimeRemaining(std::max(time_duration(seconds(0)),
          riskState.m_expiry - currentTime));
      }
      m_dialog = std::make_unique<RiskTimerDialog>();
      m_dialog->setWindowFlags(
        Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
      m_dialog->SetModel(m_model);
      m_dialog->setFixedSize(m_dialog->size());
      m_dialog->move(m_userProfile->GetRiskTimerProperties().
        GetRiskTimerDialogInitialPosition());
      m_dialog->show();
    }
  }
}
