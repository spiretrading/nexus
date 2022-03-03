#include "Spire/Styles/Rule.hpp"
#include "Spire/Styles/Any.hpp"

using namespace Spire;
using namespace Spire::Styles;

Rule::Rule()
  : Rule(Any()) {}

Rule::Rule(Block block)
  : Rule(Any(), std::move(block)) {}

Rule::Rule(Selector selector)
  : Rule(selector, {}) {}

Rule::Rule(Selector selector, Block block)
  : m_selector(std::move(selector)),
    m_block(std::move(block)) {}

const Selector& Rule::get_selector() const {
  return m_selector;
}

const Block& Rule::get_block() const {
  return m_block;
}

Block& Rule::get_block() {
  return m_block;
}

void Rule::clear() {
  m_block.clear();
}

Rule& Rule::set(Property property) {
  get_block().set(std::move(property));
  return *this;
}
