#include "Spire/AccountViewer/AccountEntitlementModel.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

AccountEntitlementModel::AccountEntitlementModel(
    Ref<UserProfile> userProfile, const DirectoryEntry& account)
    : m_userProfile(userProfile.Get()),
      m_account(account) {}

void AccountEntitlementModel::Load() {
  while(!m_entitlementsGranted.empty()) {
    DirectoryEntry entry = *m_entitlementsGranted.begin();
    Revoke(entry);
  }
  auto entitlements = m_userProfile->GetServiceClients().
    GetAdministrationClient().LoadEntitlements(m_account);
  for(const DirectoryEntry& entitlement : entitlements) {
    Grant(entitlement);
  }
}

void AccountEntitlementModel::Commit() {
  vector<DirectoryEntry> entitlements(m_entitlementsGranted.begin(),
    m_entitlementsGranted.end());
  m_userProfile->GetServiceClients().GetAdministrationClient().
    StoreEntitlements(m_account, entitlements);
}

bool AccountEntitlementModel::HasEntitlement(
    const DirectoryEntry& entry) const {
  return m_entitlementsGranted.find(entry) != m_entitlementsGranted.end();
}

void AccountEntitlementModel::Grant(const DirectoryEntry& entry) {
  if(!m_entitlementsGranted.insert(entry).second) {
    return;
  }
  m_entitlementGrantedSignal(entry);
}

void AccountEntitlementModel::Revoke(const DirectoryEntry& entry) {
  if(m_entitlementsGranted.erase(entry) == 0) {
    return;
  }
  m_entitlementRevokedSignal(entry);
}

connection AccountEntitlementModel::ConnectEntitlementGrantedSignal(
    const EntitlementGrantedSignal::slot_type& slot) const {
  return m_entitlementGrantedSignal.connect(slot);
}

connection AccountEntitlementModel::ConnectEntitlementRevokedSignal(
    const EntitlementRevokedSignal::slot_type& slot) const {
  return m_entitlementRevokedSignal.connect(slot);
}
