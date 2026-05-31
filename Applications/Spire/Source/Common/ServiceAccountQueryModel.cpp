#include "Spire/Spire/ServiceAccountQueryModel.hpp"
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
      accounts.push_back({make_identicon(result.m_account, scale(8, 8)),
        QString::fromStdString(result.m_account.m_name),
        QString::fromStdString(result.m_name)});
    }
    return accounts;
  }, LaunchPolicy::ASYNC);
}
