#include "Spire/Styles/Block.hpp"

using namespace Spire;
using namespace Spire::Styles;

const std::vector<Property>& Block::get_properties() const& {
  return m_properties;
}

std::vector<Property>&& Block::get_properties() && {
  return std::move(m_properties);
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

bool Block::operator ==(const Block& block) const {
  return m_properties == block.m_properties;
}

bool Block::operator !=(const Block& block) const {
  return !(*this == block);
}

void Spire::Styles::merge(Block& block, Block other) {
  for(auto& property : std::move(other).get_properties()) {
    block.set(std::move(property));
  }
}
