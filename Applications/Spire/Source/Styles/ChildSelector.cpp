#include "Spire/Styles/ChildSelector.hpp"
#include <QChildEvent>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct ChildObserver : public QObject {
    SelectionUpdateSignal m_on_update;

    ChildObserver(
        const Stylist& stylist, const SelectionUpdateSignal& on_update)
        : m_on_update(on_update) {
      auto children = std::unordered_set<const Stylist*>();
      for(auto child : stylist.get_widget().children()) {
        if(child->isWidgetType()) {
          children.insert(&find_stylist(static_cast<QWidget&>(*child)));
        }
      }
      if(!children.empty()) {
        m_on_update(std::move(children), {});
      }
      stylist.get_widget().installEventFilter(this);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ChildAdded) {
        auto& child_event = static_cast<QChildEvent&>(*event);
        if(child_event.child()->isWidgetType()) {
          m_on_update(
            {&find_stylist(static_cast<const QWidget&>(*child_event.child()))},
            {});
        }
      } else if(event->type() == QEvent::ChildRemoved) {
        auto& child_event = static_cast<QChildEvent&>(*event);
        if(child_event.child()->isWidgetType()) {
          m_on_update({},
            {&find_stylist(static_cast<const QWidget&>(*child_event.child()))});
        }
      }
      return QObject::eventFilter(watched, event);
    }
  };
}

ChildSelector::ChildSelector(Selector base, Selector child)
  : m_base(std::move(base)),
    m_child(std::move(child)) {}

const Selector& ChildSelector::get_base() const {
  return m_base;
}

const Selector& ChildSelector::get_child() const {
  return m_child;
}

bool ChildSelector::operator ==(const ChildSelector& selector) const {
  return m_base == selector.get_base() && m_child == selector.get_child();
}

bool ChildSelector::operator !=(const ChildSelector& selector) const {
  return !(*this == selector);
}

SelectConnection Spire::Styles::select(const ChildSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(), selector.get_child(),
    [] (const auto& stylist, const auto& on_update) {
      return SelectConnection(
        std::make_unique<ChildObserver>(stylist, on_update));
    }), base, on_update);
}
