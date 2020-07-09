#include "Spire/KeyBindings/RegionTerm.hpp"
#include <algorithm>

using namespace Nexus;
using namespace Spire;

RegionTerm::RegionTerm(Region region)
  : m_region(std::move(region)) {}

bool RegionTerm::is_satisfied(const std::vector<KeyBindings::Tag>&
    tags) const {
  auto it = std::find_if(tags.begin(), tags.end(), [] (auto& tag) {
    return tag.get_key() == 55;
  });
  if(it != tags.end()) {
    auto value = it->get_value<Region>();
    if(value) {
      return *value <= m_region;
    }
  }
  return false;
}
