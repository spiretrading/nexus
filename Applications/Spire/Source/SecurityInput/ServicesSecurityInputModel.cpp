#include "Spire/SecurityInput/ServicesSecurityInputModel.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;

ServicesSecurityInputModel::ServicesSecurityInputModel(
    Ref<VirtualMarketDataClient> client)
    : m_client(client.Get()) {}

QtPromise<std::vector<SecurityInfo>> ServicesSecurityInputModel::autocomplete(
    const std::string& query) {
  return QtPromise(
    [=, client = m_client] {
      try {
        return client->LoadSecurityInfoFromPrefix(query);
      } catch(const std::exception&) {
        return std::vector<SecurityInfo>();
      }
    }, LaunchPolicy::ASYNC);
}
