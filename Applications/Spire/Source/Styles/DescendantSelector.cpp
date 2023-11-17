#include "Spire/Styles/DescendantSelector.hpp"
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include <deque>
#include <QChildEvent>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Beam::SignalHandling;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto build_descendants(const QWidget& root) {
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

  struct DescendantObserver : public QObject {
    SelectionUpdateSignal m_on_update;
    ConnectionGroup m_delete_connections;

    DescendantObserver(
        const Stylist& stylist, const SelectionUpdateSignal& on_update)
        : m_on_update(on_update) {
      stylist.get_widget().installEventFilter(this);
      auto descendants = build_descendants(stylist.get_widget());
      for(auto descendant : descendants) {
        auto connection = descendant->connect_delete_signal(std::bind_front(
          &DescendantObserver::on_delete, this, std::ref(*descendant)));
        m_delete_connections.AddConnection(&descendant, connection);
        descendant->get_widget().installEventFilter(this);
      }
      m_on_update(std::move(descendants), {});
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ChildAdded) {
        auto& child_event = static_cast<QChildEvent&>(*event);
        if(child_event.child()->isWidgetType()) {
          auto& child = static_cast<const QWidget&>(*child_event.child());
          auto descendants = build_descendants(child);
          descendants.insert(&find_stylist(child));
          for(auto descendant : descendants) {
            auto connection = descendant->connect_delete_signal(std::bind_front(
              &DescendantObserver::on_delete, this, std::ref(*descendant)));
            m_delete_connections.AddConnection(&descendant, connection);
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
            m_delete_connections.Disconnect(&descendant);
            descendant->get_widget().removeEventFilter(this);
          }
          m_on_update({}, std::move(descendants));
        }
      }
      return QObject::eventFilter(watched, event);
    }

    void on_delete(const Stylist& stylist) {
      auto descendants = build_descendants(stylist.get_widget());
      descendants.insert(&stylist);
      for(auto descendant : descendants) {
        m_delete_connections.Disconnect(&descendant);
        descendant->get_widget().removeEventFilter(this);
      }
      m_on_update({}, std::move(descendants));
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
