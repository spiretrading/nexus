#include "Spire/Styles/DescendantSelector.hpp"
#include <deque>
#include <QChildEvent>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct DescendantObserver : public QObject {
    SelectionUpdateSignal m_on_update;

    DescendantObserver(
        const Stylist& stylist, const SelectionUpdateSignal& on_update)
        : m_on_update(on_update) {
      auto descendants = build_descendants(stylist.get_widget());
      stylist.get_widget().installEventFilter(this);
      for(auto descendant : descendants) {
        descendant->get_widget().installEventFilter(this);
      }
      m_on_update(std::move(descendants), {});
    }

    std::unordered_set<const Stylist*> build_descendants(
        const QWidget& root) const {
      auto descendants = std::unordered_set<const Stylist*>();
      auto breadth_traversal = std::deque<const QWidget*>();
      breadth_traversal.push_back(&root);
      while(!breadth_traversal.empty()) {
        auto widget = breadth_traversal.front();
        breadth_traversal.pop_front();
        for(auto child : widget->children()) {
          if(child->isWidgetType()) {
            auto& child_widget = static_cast<QWidget&>(*child);
            breadth_traversal.push_back(&child_widget);
            descendants.insert(&find_stylist(child_widget));
          }
        }
      }
      return descendants;
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ChildAdded) {
        auto& child_event = static_cast<QChildEvent&>(*event);
        if(child_event.child()->isWidgetType()) {
          auto& child = static_cast<const QWidget&>(*child_event.child());
          auto descendants = build_descendants(child);
          descendants.insert(&find_stylist(child));
          for(auto descendant : descendants) {
            descendant->get_widget().installEventFilter(this);
          }
          m_on_update(std::move(descendants), {});
        }
      } else if(event->type() == QEvent::ChildRemoved) {
        auto& child_event = static_cast<QChildEvent&>(*event);
        if(child_event.child()->isWidgetType()) {
          auto& child = static_cast<const QWidget&>(*child_event.child());
          auto descendants = build_descendants(child);
          descendants.insert(&find_stylist(child));
          for(auto descendant : descendants) {
            descendant->get_widget().removeEventFilter(this);
          }
          m_on_update({}, std::move(descendants));
        }
      }
      return QObject::eventFilter(watched, event);
    }
  };
}

DescendantSelector::DescendantSelector(Selector base, Selector descendant)
  : m_base(std::move(base)),
    m_descendant(std::move(descendant)) {}

const Selector& DescendantSelector::get_base() const {
  return m_base;
}

const Selector& DescendantSelector::get_descendant() const {
  return m_descendant;
}

bool DescendantSelector::operator ==(const DescendantSelector& selector) const {
  return m_base == selector.get_base() &&
    m_descendant == selector.get_descendant();
}

bool DescendantSelector::operator !=(const DescendantSelector& selector) const {
  return !(*this == selector);
}

DescendantSelector Spire::Styles::operator >>(
    Selector base, Selector descendant) {
  return DescendantSelector(std::move(base), std::move(descendant));
}

SelectConnection Spire::Styles::select(const DescendantSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(),
    selector.get_descendant(), [] (const auto& stylist, const auto& on_update) {
      return SelectConnection(
        std::make_unique<DescendantObserver>(stylist, on_update));
    }), base, on_update);
}