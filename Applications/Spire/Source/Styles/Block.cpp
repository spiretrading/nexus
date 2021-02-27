#include "Spire/Styles/Block.hpp"

using namespace Spire;
using namespace Spire::Styles;

const std::vector<Property>& Block::get_properties() const {
  return m_properties;
}

void Block::set(Property property) {
  for(auto& p : m_properties) {
    if(p.get_type() == property.get_type()) {
      p = std::move(property);
      return;
    }
  }
  m_properties.push_back(std::move(property));
}

void Block::remove(const Property& property) {
  auto i = std::find_if(m_properties.begin(), m_properties.end(),
    [&] (const auto& p) {
      return p.get_type() == property.get_type();
    });
  if(i != m_properties.end()) {
    m_properties.erase(i);
  }
}
