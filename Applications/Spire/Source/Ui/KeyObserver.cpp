#include "Spire/Ui/KeyObserver.hpp"
#include "Spire/Spire/ExtensionCache.hpp"
#include <QWidget>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

struct KeyObserver::EventFilter : QObject {
  struct Child {
    std::unique_ptr<KeyObserver> m_observer;
    scoped_connection m_connection;
  };
  mutable FilteredKeyPressSignal m_key_press_signal;
  std::unordered_map<QObject*, Child> m_children;

  EventFilter(QWidget& widget) {
    widget.installEventFilter(this);
    for(auto child : widget.children()) {
      if(child && child->isWidgetType()) {
        add(static_cast<QWidget&>(*child));
      }
    }
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::KeyPress) {
      auto filter = m_key_press_signal(*static_cast<QWidget*>(watched),
        *static_cast<QKeyEvent*>(event)).value_or(false);
      return filter;
    } else if(event->type() == QEvent::ChildAdded) {
      auto& child = *static_cast<QChildEvent&>(*event).child();
      if(child.isWidgetType()) {
        add(static_cast<QWidget&>(child));
      }
    } else if(event->type() == QEvent::ChildRemoved) {
      auto& child_event = static_cast<QChildEvent&>(*event);
      m_children.erase(child_event.child());
    }
    return QObject::eventFilter(watched, event);
  }

  void add(QWidget& child) {
    auto observer = std::make_unique<KeyObserver>(child);
    auto connection = observer->connect_filtered_key_press_signal(
      [=] (auto& target, auto& event) {
        return m_key_press_signal(target, event).value_or(false);
      });
    auto child_entry = Child(std::move(observer), std::move(connection));
    m_children.insert(std::pair(&child, std::move(child_entry)));
  }
};

KeyObserver::KeyObserver(QWidget& widget) {
  m_filter = find_extension<EventFilter>(widget);
  m_filter_connection = m_filter->m_key_press_signal.connect(
    [=] (auto& target, auto& event) {
      return m_key_press_signal(target, event).value_or(false);
    });
}

connection KeyObserver::connect_filtered_key_press_signal(
    const FilteredKeyPressSignal::slot_type& slot) const {
  return m_key_press_signal.connect(slot);
}

connection KeyObserver::connect_key_press_signal(
    const KeyPressSignal& slot) const {
  return connect_filtered_key_press_signal([=] (auto& target, auto& event) {
    slot(target, event);
    return false;
  });
}
