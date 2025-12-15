#include "Spire/AccountViewer/RiskModel.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

RiskModel::RiskModel(
  Ref<UserProfile> userProfile, const DirectoryEntry& account)
  : m_userProfile(userProfile.get()),
    m_account(account) {}

void RiskModel::Load() {
  auto queue = std::make_shared<Queue<RiskParameters>>();
  m_userProfile->GetClients().get_administration_client().
    get_risk_parameters_publisher(m_account).monitor(queue);
  m_riskParameters = queue->pop();
}

void RiskModel::Commit() {
  m_userProfile->GetClients().get_administration_client().store(
    m_account, m_riskParameters);
}

const DirectoryEntry& RiskModel::GetAccount() const {
  return m_account;
}

const RiskParameters& RiskModel::GetRiskParameters() const {
  return m_riskParameters;
}

RiskParameters& RiskModel::GetRiskParameters() {
  return m_riskParameters;
}
