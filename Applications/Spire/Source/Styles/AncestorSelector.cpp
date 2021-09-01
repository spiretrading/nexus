#include "Spire/Styles/AncestorSelector.hpp"
#include <deque>
#include <unordered_map>
#include <QWidget>
#include "Spire/Styles/FlipSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

AncestorSelector::AncestorSelector(Selector base, Selector ancestor)
  : m_base(std::move(base)),
    m_ancestor(std::move(ancestor)) {}

const Selector& AncestorSelector::get_base() const {
  return m_base;
}

const Selector& AncestorSelector::get_ancestor() const {
  return m_ancestor;
}

bool AncestorSelector::operator ==(const AncestorSelector& selector) const {
  return m_base == selector.get_base() && m_ancestor == selector.get_ancestor();
}

bool AncestorSelector::operator !=(const AncestorSelector& selector) const {
  return !(*this == selector);
}

AncestorSelector Spire::Styles::operator <<(Selector base, Selector ancestor) {
  return AncestorSelector(std::move(base), std::move(ancestor));
}

SelectConnection Spire::Styles::select(const AncestorSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  struct Executor {
    struct AncestorEntry {
      int m_count;
      SelectConnection m_connection;
      std::unordered_set<const Stylist*> m_selection;

      AncestorEntry()
        : m_count(0) {}
    };
    Selector m_ancestor_selector;
    SelectionUpdateSignal m_on_update;
    SelectConnection m_base_connection;
    std::unordered_map<const Stylist*, AncestorEntry> m_ancestors;
    std::unordered_map<const Stylist*, int> m_selection;

    Executor(const AncestorSelector& selector, const Stylist& base,
      const SelectionUpdateSignal& on_update)
      : m_ancestor_selector(selector.get_ancestor()),
        m_on_update(on_update),
        m_base_connection(select(selector.get_base(), base,
          std::bind_front(&Executor::on_base, this))) {}

    void on_base(std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      for(auto addition : additions) {
        auto ancestor = addition->get_widget().parentWidget();
        while(ancestor) {
          auto& stylist = find_stylist(*ancestor);
          auto& entry = m_ancestors[&stylist];
          ++entry.m_count;
          if(entry.m_count == 1) {
            entry.m_connection = select(m_ancestor_selector, stylist,
              std::bind_front(&Executor::on_ancestor, this, std::ref(entry)));
          }
          ancestor = ancestor->parentWidget();
        }
      }
      for(auto removal : removals) {
        auto ancestor = removal->get_widget().parentWidget();
        while(ancestor) {
          auto& stylist = find_stylist(*ancestor);
          auto& entry = m_ancestors[&stylist];
          --entry.m_count;
          if(entry.m_count == 0) {
            auto removals = std::move(entry.m_selection);
            entry.m_selection.clear();
            on_ancestor(entry, {}, std::move(removals));
            m_ancestors.erase(&stylist);
          }
          ancestor = ancestor->parentWidget();
        }
      }
    }

    void on_ancestor(
        AncestorEntry& entry, std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      for(auto i = additions.begin(); i != additions.end();) {
        entry.m_selection.insert(*i);
        auto& count = m_selection[*i];
        ++count;
        if(count != 1) {
          i = additions.erase(i);
        } else {
          ++i;
        }
      }
      for(auto i = removals.begin(); i != removals.end();) {
        entry.m_selection.erase(*i);
        auto& count = m_selection[*i];
        --count;
        if(count != 0) {
          i = removals.erase(i);
        } else {
          ++i;
        }
      }
      if(!additions.empty() || !removals.empty()) {
        m_on_update(std::move(additions), std::move(removals));
      }
    }
  };
  return SelectConnection(
    std::make_unique<Executor>(selector, base, on_update));
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const AncestorSelector& selector, std::unordered_set<Stylist*> sources) {
  auto is_flipped = selector.get_base().get_type() == typeid(FlipSelector);
  auto selection = std::unordered_set<Stylist*>();
  for(auto source : select(selector.get_base(), std::move(sources))) {
    auto ancestor = source->get_widget().parentWidget();
    while(ancestor) {
      auto ancestor_selection =
        select(selector.get_ancestor(), find_stylist(*ancestor));
      if(!ancestor_selection.empty()) {
        if(is_flipped) {
          selection.insert(source);
          break;
        } else {
          selection.insert(
            ancestor_selection.begin(), ancestor_selection.end());
        }
      }
      ancestor = ancestor->parentWidget();
    }
  }
  return selection;
}

std::unordered_set<QWidget*> Spire::Styles::build_reach(
    const AncestorSelector& selector, QWidget& source) {
  auto reach = build_reach(selector.get_base(), source);
  auto ancestors = std::deque<QWidget*>();
  for(auto base : reach) {
    if(auto parent = base->parentWidget()) {
      ancestors.push_back(parent);
    }
  }
  while(!ancestors.empty()) {
    auto ancestor = ancestors.front();
    ancestors.pop_front();
    auto ancestor_reach = build_reach(selector.get_ancestor(), *ancestor);
    reach.insert(ancestor_reach.begin(), ancestor_reach.end());
    if(auto parent = ancestor->parentWidget()) {
      ancestors.push_back(parent);
    }
  }
  return reach;
}
