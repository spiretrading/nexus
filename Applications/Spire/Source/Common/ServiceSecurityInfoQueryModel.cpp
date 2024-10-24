#include "Spire/Spire/ServiceSecurityInfoQueryModel.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;

ServiceSecurityInfoQueryModel::ServiceSecurityInfoQueryModel(
  MarketDatabase markets, MarketDataClientBox market_data_client)
  : m_markets(std::move(markets)),
    m_market_data_client(std::move(market_data_client)) {}

const MarketDatabase& ServiceSecurityInfoQueryModel::get_markets() const {
  return m_markets;
}

SecurityInfo ServiceSecurityInfoQueryModel::parse_security(
    const QString& query) {
  auto security = ParseSecurity(query.toUpper().toStdString(), m_markets);
  if(security == Security()) {
    return {};
  }
  if(auto info = m_info.FindValue(security)) {
    return *info;
  }
  auto info = SecurityInfo();
  info.m_security = std::move(security);
  return info;
}

QtPromise<std::vector<SecurityInfo>>
    ServiceSecurityInfoQueryModel::submit_security(const QString& query) {
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
