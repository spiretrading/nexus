#include "Spire/Spire/SecurityInfoQueryModel.hpp"

using namespace Nexus;
using namespace Spire;

std::any SecurityInfoQueryModel::parse(const QString& query) {
  auto security = parse_security(query);
  if(security.m_security == Security()) {
    return {};
  }
  return security;
}

QtPromise<std::vector<std::any>>
    SecurityInfoQueryModel::submit(const QString& query) {
  return submit_security(query).then([] (std::vector<SecurityInfo> securities) {
    return std::vector<std::any>(std::make_move_iterator(securities.begin()),
      std::make_move_iterator(securities.end()));
  });
}
