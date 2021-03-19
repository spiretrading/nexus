#include "Spire/Styles/StyleSheet.hpp"
#include "Spire/Styles/Any.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

StyleSheet::StyleSheet() {
  m_rules.emplace_back(Any(), Block());
}

const std::vector<Rule>& StyleSheet::get_rules() const {
  return m_rules;
}

optional<const Rule&> StyleSheet::find(const Selector& selector) const {
  for(auto& rule : m_rules) {
    if(rule.get_selector().is_match(selector)) {
      return rule;
    }
  }
  return none;
}

Rule& StyleSheet::get(const Selector& selector) {
  auto rule = find(selector);
  if(rule) {
    return const_cast<Rule&>(*rule);
  }
  m_rules.emplace_back(selector, Block());
  return m_rules.back();
}
