#include "Spire/KeyBindings/AdditionalTagDatabase.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  const auto NONE = std::shared_ptr<AdditionalTagSchema>();
}

const std::shared_ptr<AdditionalTagSchema>&
    AdditionalTagDatabase::find(const Region& region, int key) const {
  return NONE;
}

const std::shared_ptr<AdditionalTagSchema>&
    AdditionalTagDatabase::find(const Destination& destination, int key) const {
  return NONE;
}

const std::shared_ptr<AdditionalTagSchema>& Spire::find(
    const AdditionalTagDatabase& database,
    const Nexus::Destination& destination, const Nexus::Region& region,
    int key) {
  if(auto& schema = database.find(destination, key)) {
    return schema;
  }
  return database.find(region, key);
}
