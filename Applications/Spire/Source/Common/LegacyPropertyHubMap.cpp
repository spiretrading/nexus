#include "Spire/Spire/LegacyPropertyHubMap.hpp"

using namespace boost::signals2;
using namespace Spire;

std::shared_ptr<PropertyHub> LegacyPropertyHubMap::acquire(
    const std::string& identifier, const std::string& link_identifier) {
  std::erase_if(m_hubs, [] (const auto& entry) {
    return entry.second.expired();
  });
  auto hub = find(identifier);
  if(auto link_hub = find(link_identifier)) {
    if(!hub) {
      hub = link_hub;
    } else if(hub != link_hub) {
      merge(link_hub, hub);
    }
  }
  if(!hub) {
    hub = std::make_shared<PropertyHub>();
  }
  if(!identifier.empty()) {
    m_hubs[identifier] = hub;
  }
  if(!link_identifier.empty()) {
    m_hubs[link_identifier] = hub;
  }
  return hub;
}

connection LegacyPropertyHubMap::connect_merge_signal(
    const MergeSignal::slot_type& slot) const {
  return m_merge_signal.connect(slot);
}

std::shared_ptr<PropertyHub> LegacyPropertyHubMap::find(
    const std::string& identifier) const {
  if(identifier.empty()) {
    return nullptr;
  }
  auto i = m_hubs.find(identifier);
  if(i == m_hubs.end()) {
    return nullptr;
  }
  return i->second.lock();
}

void LegacyPropertyHubMap::merge(const std::shared_ptr<PropertyHub>& source,
    const std::shared_ptr<PropertyHub>& destination) {
  for(auto& entry : m_hubs) {
    if(entry.second.lock() == source) {
      entry.second = destination;
    }
  }
  m_merge_signal(source, destination);
}
