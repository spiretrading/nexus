#include "Spire/AccountViewer/AccountInfoModel.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Spire;

AccountInfoModel::AccountInfoModel(Ref<UserProfile> userProfile,
    const DirectoryEntry& account)
    : m_userProfile(userProfile.Get()),
      m_account(account) {}

void AccountInfoModel::Load() {
  m_identity = m_userProfile->GetServiceClients().GetAdministrationClient().
    LoadIdentity(m_account);
}

void AccountInfoModel::Commit() {
  m_userProfile->GetServiceClients().GetAdministrationClient().StoreIdentity(
    m_account, m_identity);
}

const DirectoryEntry& AccountInfoModel::GetAccount() const {
  return m_account;
}

const AccountIdentity& AccountInfoModel::GetIdentity() const {
  return m_identity;
}

AccountIdentity& AccountInfoModel::GetIdentity() {
  return m_identity;
}
