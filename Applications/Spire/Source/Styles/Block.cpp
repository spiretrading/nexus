#include "Spire/Styles/Block.hpp"

using namespace Spire;
using namespace Spire::Styles;

Block::Block() {}

Block::Block(Rule rule)
  : m_rule(std::move(rule)) {}

const Rule& Block::get_rule() const {
  return m_rule;
}

const std::vector<Property>& Block::get_properties() const {
  return m_properties;
}

void Block::set(Property property) {
  m_properties.push_back(std::move(property));
}
