#include "Spire/AccountViewer/AccountInfoModel.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

AccountInfoModel::AccountInfoModel(Ref<UserProfile> userProfile,
    const DirectoryEntry& account)
    : m_userProfile(userProfile.get()),
      m_account(account) {}

void AccountInfoModel::Load() {
  m_identity =
    m_userProfile->GetClients().get_administration_client().load_identity(
      m_account);
}

void AccountInfoModel::Commit() {
  m_userProfile->GetClients().get_administration_client().store(
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
