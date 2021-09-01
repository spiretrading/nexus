#include "Spire/Styles/NotSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

NotSelector::NotSelector(Selector selector)
  : m_selector(std::move(selector)) {}

const Selector& NotSelector::get_selector() const {
  return m_selector;
}

bool NotSelector::operator ==(const NotSelector& selector) const {
  return m_selector == selector.get_selector();
}

bool NotSelector::operator !=(const NotSelector& selector) const {
  return !(*this == selector);
}

NotSelector Spire::Styles::operator !(Selector selector) {
  return NotSelector(std::move(selector));
}

SelectConnection Spire::Styles::select(const NotSelector& selector,
    const Stylist& base, const SelectionUpdate& on_update) {
  struct Executor {
    const Stylist* m_base;
    SelectionUpdate m_on_update;
    SelectConnection m_connection;

    Executor(const NotSelector& selector, const Stylist& base,
        const SelectionUpdate& on_update)
        : m_base(&base) {
      auto is_included = false;
      m_on_update = [&] (
          std::unordered_set<const Stylist*>&& additions,
          std::unordered_set<const Stylist*>&& removals) {
        is_included = true;
        if(!additions.empty()) {
          on_update(std::move(additions), {});
        }
      };
      m_connection = select(selector.get_selector(), *m_base,
        std::bind_front(&Executor::on_update, this));
      m_on_update = on_update;
      if(!is_included) {
        m_on_update({m_base}, {});
      }
    }

    void on_update(
        std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      if(additions.contains(m_base)) {
        m_on_update({}, {m_base});
      } else if(removals.contains(m_base)) {
        m_on_update({m_base}, {});
      }
    }
  };
  return SelectConnection(
    std::make_unique<Executor>(selector, base, on_update));
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const NotSelector& selector, std::unordered_set<Stylist*> sources) {
  for(auto selected : select(selector.get_selector(), sources)) {
    sources.erase(selected);
  }
  return sources;
}
