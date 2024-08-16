#include "Spire/AccountViewer/RiskModel.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::RiskService;
using namespace Spire;

RiskModel::RiskModel(Ref<UserProfile> userProfile,
    const DirectoryEntry& account)
    : m_userProfile(userProfile.Get()),
      m_account(account) {}

void RiskModel::Load() {
  auto queue = std::make_shared<Queue<RiskParameters>>();
  m_userProfile->GetServiceClients().GetAdministrationClient().
    GetRiskParametersPublisher(m_account).Monitor(queue);
  m_riskParameters = queue->Pop();
}

void RiskModel::Commit() {
  m_userProfile->GetServiceClients().GetAdministrationClient().
    StoreRiskParameters(m_account, m_riskParameters);
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
