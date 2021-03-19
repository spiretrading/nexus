#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/Rule.hpp"

using namespace Spire;
using namespace Spire::Styles;

Rule::Rule()
  : Rule(Any()) {}

Rule::Rule(Block block)
  : Rule(Any(), std::move(block)) {}

Rule::Rule(Override override)
  : Rule(Any(), override) {}

Rule::Rule(Selector selector)
  : Rule(selector, {}) {}

Rule::Rule(Block block, Override override)
  : Rule(Any(), std::move(block), override) {}

Rule::Rule(Selector selector, Block block)
  : Rule(std::move(selector), std::move(block), Override::INCLUSIVE) {}

Rule::Rule(Selector selector, Override override)
  : Rule(std::move(selector), {}, override) {}

Rule::Rule(Selector selector, Block block, Override override)
  : m_selector(std::move(selector)),
    m_block(std::move(block)),
    m_override(override) {}

const Selector& Rule::get_selector() const {
  return m_selector;
}

const Block& Rule::get_block() const {
  return m_block;
}

Block& Rule::get_block() {
  return m_block;
}

Rule::Override Rule::get_override() const {
  return m_override;
}

Rule& Rule::set_override(Override override) {
  m_override = override;
  return *this;
}

Rule& Rule::set(Property property) {
  get_block().set(std::move(property));
  return *this;
}
