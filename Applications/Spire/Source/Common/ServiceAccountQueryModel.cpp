#include "Spire/Spire/ServiceAccountQueryModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Identicon.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

ServiceAccountQueryModel::ServiceAccountQueryModel(
  AdministrationClient administration_client)
  : m_administration_client(std::move(administration_client)) {}

optional<AccountListItem::Account>
    ServiceAccountQueryModel::parse(const QString& query) {
  auto lock = std::lock_guard(m_mutex);
  auto i = m_cache.find(query);
  if(i != m_cache.end()) {
    return i->second;
  }
  return none;
}

QtPromise<std::vector<AccountListItem::Account>>
    ServiceAccountQueryModel::submit(const QString& query) {
  if(query.isEmpty()) {
    return {};
  }
  return QtPromise([=, client = m_administration_client] () mutable {
    auto results = client.query_accounts(query.toStdString());
    auto accounts = std::vector<AccountListItem::Account>();
    for(auto& result : results) {
      auto account = AccountListItem::Account(
        make_identicon(result.m_account, scale(8, 8)), result.m_account,
        QString::fromStdString(result.m_name));
      auto key = QString::fromStdString(result.m_account.m_name);
      {
        auto lock = std::lock_guard(m_mutex);
        m_cache.insert_or_assign(key, account);
      }
      accounts.push_back(std::move(account));
    }
    return accounts;
  }, LaunchPolicy::ASYNC);
}
