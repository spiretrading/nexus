#include "Spire/Spire/ServiceSecurityQueryModel.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;

ServiceSecurityQueryModel::ServiceSecurityQueryModel(MarketDatabase markets,
  MarketDataClientBox market_data_client)
  : m_markets(std::move(markets)),
    m_market_data_client(std::move(market_data_client)) {}

optional<SecurityInfo> ServiceSecurityQueryModel::parse(const QString& query) {
  auto security = ParseSecurity(query.toStdString(), m_markets);
  if(security == Security()) {
    return none;
  }
  if(auto info = m_info.FindValue(security)) {
    return *info;
  }
  auto info = SecurityInfo();
  info.m_security = std::move(security);
  return info;
}

QtPromise<std::vector<SecurityInfo>>
    ServiceSecurityQueryModel::submit(const QString& query) {
  if(query.isEmpty()) {
    return {};
  }
  return QtPromise([=, client = m_market_data_client] () mutable {
    auto securities =
      client.LoadSecurityInfoFromPrefix(query.trimmed().toStdString());
    for(auto& security : securities) {
      m_info.Insert(security.m_security, security);
    }
    return securities;
  }, LaunchPolicy::ASYNC);
}
