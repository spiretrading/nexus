#include "Spire/KeyBindings/KeyBindingsModel.hpp"

using namespace Nexus;
using namespace Spire;

KeyBindingsModel::KeyBindingsModel()
    : m_order_task_arguments(
        std::make_shared<ArrayListModel<OrderTaskArguments>>()),
      m_cancel_key_bindings(std::make_shared<CancelKeyBindingsModel>()) {
  m_interactions.insert(std::pair(
    Region::make_global(QObject::tr("Global").toStdString()),
    std::make_shared<InteractionsKeyBindingsModel>()));
}

const std::shared_ptr<OrderTaskArgumentsListModel>&
    KeyBindingsModel::get_order_task_arguments() const {
  return m_order_task_arguments;
}

const std::shared_ptr<CancelKeyBindingsModel>&
    KeyBindingsModel::get_cancel_key_bindings() const {
  return m_cancel_key_bindings;
}

const std::shared_ptr<InteractionsKeyBindingsModel>&
    KeyBindingsModel::get_interactions_key_bindings(
      const Region& region) const {
  if(region.get_countries().size() == 1) {
    auto country = *region.get_countries().begin();
    auto i = m_interactions.find(country);
    if(i == m_interactions.end()) {
      auto interactions = std::make_shared<InteractionsKeyBindingsModel>(
        get_interactions_key_bindings(Region::GLOBAL));
      i = m_interactions.insert(std::pair(country, interactions)).first;
    }
    return i->second;
  } else if(region.get_venues().size() == 1) {
    auto& venue = DEFAULT_VENUES.from(*region.get_venues().begin());
    auto i = m_interactions.find(venue.m_venue);
    if(i == m_interactions.end()) {
      auto interactions = std::make_shared<InteractionsKeyBindingsModel>(
        get_interactions_key_bindings(venue.m_country_code));
      i = m_interactions.insert(std::pair(venue.m_venue, interactions)).first;
    }
    return i->second;
  } else if(region.get_securities().size() == 1) {
    auto security = *region.get_securities().begin();
    auto i = m_interactions.find(security);
    if(i == m_interactions.end()) {
      auto interactions = std::make_shared<InteractionsKeyBindingsModel>(
        get_interactions_key_bindings(security.get_venue()));
      i = m_interactions.insert(std::pair(security, interactions)).first;
    }
    return i->second;
  } else if(region.is_global()) {
    return m_interactions.at(region);
  }
  static auto NONE = std::shared_ptr<InteractionsKeyBindingsModel>();
  return NONE;
}

std::vector<Region>
    KeyBindingsModel::make_interactions_key_bindings_regions() const {
  auto regions = std::vector<Region>();
  for(auto& entry : m_interactions) {
    if(entry.second->is_detached()) {
      regions.push_back(entry.first);
    }
  }
  return regions;
}
