#include "Spire/Styles/AncestorSelector.hpp"
#include <QEvent>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct AncestorObserver : public QObject {
    std::function<void (std::unordered_set<const Stylist*>&& ancestors)>
      m_on_ancestors_added;

    AncestorObserver(const Stylist& stylist,
        std::function<void (std::unordered_set<const Stylist*>&&)>&&
          on_ancestors_added)
        : m_on_ancestors_added(std::move(on_ancestors_added)) {
      auto ancestors = build_ancestors(stylist.get_widget());
      const_cast<Stylist&>(stylist).get_widget().installEventFilter(this);
      for(auto ancestor : ancestors) {
        const_cast<Stylist&>(*ancestor).get_widget().installEventFilter(this);
      }
      m_on_ancestors_added(std::move(ancestors));
    }

    std::unordered_set<const Stylist*> build_ancestors(
        const QWidget& root) const {
      auto ancestors = std::unordered_set<const Stylist*>();
      auto ancestor = root.parentWidget();
      while(ancestor) {
        ancestors.insert(&find_stylist(*ancestor));
        ancestor = ancestor->parentWidget();
      }
      return ancestors;
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ParentChange) {
        auto ancestors = build_ancestors(static_cast<QWidget&>(*watched));
        for(auto ancestor : ancestors) {
          const_cast<Stylist&>(*ancestor).get_widget().installEventFilter(this);
        }
        m_on_ancestors_added(std::move(ancestors));
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
  return select(CombinatorSelector(selector.get_base(), selector.get_ancestor(),
    [] (const Stylist& stylist, const SelectionUpdateSignal& on_update) {
      return SelectConnection(std::make_unique<AncestorObserver>(stylist,
        [=] (std::unordered_set<const Stylist*>&& ancestors) {
          on_update(std::move(ancestors), {});
        }));
     }), base, on_update);
}
