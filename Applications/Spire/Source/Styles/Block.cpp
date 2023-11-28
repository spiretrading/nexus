#include "Spire/Styles/Block.hpp"
#include <boost/functional/hash.hpp>

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

std::vector<Property>::const_iterator Block::begin() const {
  return m_properties.cbegin();
}

std::vector<Property>::const_iterator Block::end() const {
  return m_properties.cend();
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

void Block::clear() {
  m_properties.clear();
}

bool Block::operator ==(const Block& block) const {
  return m_properties == block.m_properties;
}

bool Block::operator !=(const Block& block) const {
  return !(*this == block);
}

optional<Property>
    Spire::Styles::find(const Block& block, const std::type_index& type) {
  for(auto& property : block) {
    if(property.get_type() == type) {
      return property;
    }
  }
  return none;
}

void Spire::Styles::merge(Block& block, Block other) {
  for(auto& property : other) {
    block.set(std::move(property));
  }
}

std::size_t std::hash<Block>::operator ()(const Block& block) const {
  auto seed = std::size_t(std::distance(block.begin(), block.end()));
  for(auto& property : block) {
    hash_combine(seed, std::hash<Property>()(property));
  }
  return seed;
}
