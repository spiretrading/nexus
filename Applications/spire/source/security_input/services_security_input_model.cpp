#include "spire/security_input/services_security_input_model.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;

ServicesSecurityInputModel::ServicesSecurityInputModel(
    Ref<VirtualMarketDataClient> client)
    : m_client(client.Get()) {}

std::vector<SecurityInfo> ServicesSecurityInputModel::autocomplete(
    const std::string& query) {
  try {
    return m_client->LoadSecurityInfoFromPrefix(query);
  } catch(const std::exception&) {
    return {};
  }
}
