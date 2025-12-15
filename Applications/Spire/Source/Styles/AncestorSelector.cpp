#include "Spire/Styles/AncestorSelector.hpp"
#include <boost/functional/hash.hpp>
#include <QEvent>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto build_ancestors(const QWidget& descendant) {
    auto ancestors = std::unordered_set<const Stylist*>();
    auto ancestor = &descendant;
    while(ancestor = ancestor->parentWidget()) {
      ancestors.insert(&find_stylist(*ancestor));
    }
    return ancestors;
  }

  struct AncestorObserver : public QObject {
    SelectionUpdateSignal m_on_update;
    std::unordered_map<const Stylist*, const Stylist*> m_parents;

    AncestorObserver(
        const Stylist& stylist, const SelectionUpdateSignal& on_update)
        : m_on_update(on_update) {
      auto ancestors = build_ancestors(stylist.get_widget());
      for(auto& ancestor : ancestors) {
        if(auto parent = find_parent(*ancestor)) {
          m_parents[ancestor] = parent;
        }
        ancestor->get_widget().installEventFilter(this);
      }
      m_on_update(std::move(ancestors), {});
      if(auto parent = find_parent(stylist)) {
        m_parents[&stylist] = parent;
      }
      stylist.get_widget().installEventFilter(this);
    }

    bool is_connected() const {
      return true;
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ParentChange) {
        auto& widget = static_cast<const QWidget&>(*watched);
        auto& stylist = find_stylist(widget);
        auto parent = m_parents.find(&stylist);
        if(parent == m_parents.end()) {
          return QObject::eventFilter(watched, event);
        }
        auto removals = build_ancestors(parent->second->get_widget());
        removals.insert(parent->second);
        if(auto parent = find_parent(find_stylist(widget))) {
          m_parents[&stylist] = parent;
        } else {
          m_parents.erase(&stylist);
        }
        auto ancestors = build_ancestors(widget);
        auto [base, complement] = [&] {
          if(ancestors.size() <= removals.size()) {
            return std::tuple{&ancestors, &removals};
          }
          return std::tuple{&removals, &ancestors};
        }();
        for(auto i = base->begin(); i != base->end();) {
          auto stylist = *i;
          if(complement->contains(stylist)) {
            complement->erase(stylist);
            i = base->erase(i);
          } else {
            ++i;
          }
        }
        for(auto removal : removals) {
          removal->get_widget().removeEventFilter(this);
        }
        for(auto ancestor : ancestors) {
          ancestor->get_widget().installEventFilter(this);
        }
        m_on_update(std::move(ancestors), std::move(removals));
      }
      return QObject::eventFilter(watched, event);
    }
  };
}

AncestorSelector::AncestorSelector(Selector base, Selector ancestor)
  : m_base(std::move(base)),
    m_ancestor(std::move(ancestor)) {}

const Selector& AncestorSelector::get_base() const {
  return m_base;
}

const Selector& AncestorSelector::get_ancestor() const {
  return m_ancestor;
}

AncestorSelector Spire::Styles::operator <<(Selector base, Selector ancestor) {
  return AncestorSelector(std::move(base), std::move(ancestor));
}

SelectConnection Spire::Styles::select(const AncestorSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(), selector.get_ancestor(),
    [] (const auto& stylist, const auto& on_update) {
      return SelectConnection(
        std::make_unique<AncestorObserver>(stylist, on_update));
    }), base, on_update);
}

std::size_t std::hash<AncestorSelector>::operator ()(
    const AncestorSelector& selector) const noexcept {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<Selector>()(selector.get_base()));
  hash_combine(seed, std::hash<Selector>()(selector.get_ancestor()));
  return seed;
}
