#include "Spire/Spire/LocalSecurityQueryModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;

LocalSecurityQueryModel::LocalSecurityQueryModel(MarketDatabase markets)
  : m_markets(std::move(markets)),
    m_values(QChar()) {}

void LocalSecurityQueryModel::add(const SecurityInfo& security) {
  m_values[to_text(security.m_security).toLower().data()] = security;
  m_values[QString::fromStdString(security.m_name).toLower().data()] =
    security;
}

const MarketDatabase& LocalSecurityQueryModel::get_markets() const {
  return m_markets;
}

SecurityInfo LocalSecurityQueryModel::parse_security(const QString& query) {
  auto i = m_values.find(query.toLower().data());
  if(i == m_values.end()) {
    return {};
  }
  return *i->second;
}

QtPromise<std::vector<SecurityInfo>>
    LocalSecurityQueryModel::submit_security(const QString& query) {
  auto matches = std::vector<SecurityInfo>();
  for(auto i = m_values.startsWith(query.toLower().data());
      i != m_values.end(); ++i) {
    matches.push_back(*i->second);
  }
  return QtPromise(std::move(matches));
}
