#include "Spire/Ui/MouseMoveObserver.hpp"
#include "Spire/Spire/ExtensionCache.hpp"
#include <QWidget>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

struct MouseMoveObserver::EventFilter : QObject {
  struct Child {
    std::unique_ptr<MouseMoveObserver> m_observer;
    scoped_connection m_connection;
  };
  mutable MoveSignal m_move_signal;
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
    if(event->type() == QEvent::MouseMove) {
      m_move_signal(
        *static_cast<QWidget*>(watched), *static_cast<QMouseEvent*>(event));
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
    auto observer = std::make_unique<MouseMoveObserver>(child);
    auto connection = observer->connect_move_signal(
      [=] (auto& target, auto& event) {
        return m_move_signal(target, event);
      });
    auto child_entry = Child(std::move(observer), std::move(connection));
    m_children.insert(std::pair(&child, std::move(child_entry)));
  }
};

MouseMoveObserver::MouseMoveObserver(QWidget& widget) {
  widget.setMouseTracking(true);
  m_filter = find_extension<EventFilter>(widget);
  m_connection = m_filter->m_move_signal.connect(
    [=] (auto& target, auto& event) {
      m_move_signal(target, event);
    });
}

connection MouseMoveObserver::connect_move_signal(
    const MoveSignal::slot_type& slot) const {
  return m_move_signal.connect(slot);
}
