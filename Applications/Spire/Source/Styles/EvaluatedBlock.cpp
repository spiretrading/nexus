#include "Spire/Styles/EvaluatedBlock.hpp"

using namespace Spire;
using namespace Spire::Styles;

std::vector<EvaluatedProperty>::const_iterator EvaluatedBlock::begin() const {
  return m_properties.begin();
}

std::vector<EvaluatedProperty>::const_iterator EvaluatedBlock::end() const {
  return m_properties.end();
}

Block EvaluatedBlock::to_block() const {
  auto block = Block();
  for(auto& property : *this) {
    block.set(property.as_property());
  }
  return block;
}

void EvaluatedBlock::set(EvaluatedProperty property) {
  for(auto& p : m_properties) {
    if(p.get_property_type() == property.get_property_type()) {
      p = std::move(property);
      return;
    }
  }
  m_properties.push_back(std::move(property));
}

void EvaluatedBlock::remove(const EvaluatedProperty& property) {
  auto i = std::find_if(m_properties.begin(), m_properties.end(),
    [&] (const auto& p) {
      return p.get_property_type() == property.get_property_type();
    });
  if(i != m_properties.end()) {
    m_properties.erase(i);
  }
}

void EvaluatedBlock::remove(const Property& property) {
  auto i = std::find_if(m_properties.begin(), m_properties.end(),
    [&] (const auto& p) {
      return p.get_property_type() == property.get_type();
    });
  if(i != m_properties.end()) {
    m_properties.erase(i);
  }
}
