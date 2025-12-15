#include "Spire/Spire/ServiceSecurityInfoQueryModel.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

ServiceSecurityInfoQueryModel::ServiceSecurityInfoQueryModel(
  MarketDataClient market_data_client)
  : m_market_data_client(std::move(market_data_client)) {}

optional<SecurityInfo> ServiceSecurityInfoQueryModel::parse(
    const QString& query) {
  auto security = parse_security(query.toStdString());
  if(!security) {
    return none;
  }
  if(auto info = m_info.try_load(security)) {
    return *info;
  }
  auto info = SecurityInfo();
  info.m_security = std::move(security);
  return info;
}

QtPromise<std::vector<SecurityInfo>>
    ServiceSecurityInfoQueryModel::submit(const QString& query) {
  if(query.isEmpty()) {
    return {};
  }
  return QtPromise([=, client = m_market_data_client] () mutable {
    auto securities =
      client.load_security_info_from_prefix(query.trimmed().toStdString());
    for(auto& security : securities) {
      m_info.insert(security.m_security, security);
    }
    return securities;
  }, LaunchPolicy::ASYNC);
}
