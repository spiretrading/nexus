#include "Spire/Ui/MouseObserver.hpp"
#include "Spire/Spire/ExtensionCache.hpp"
#include <QWidget>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  struct MouseCombiner {
    using result_type = bool;

    template<typename Iterator>
    result_type operator()(Iterator first, Iterator last) const {
      for(auto i = first; i != last; ++i) {
        if(*i) {
          return true;
        }
      }
      return false;
    }
  };
}

struct MouseObserver::EventFilter : QObject {
  struct Child {
    std::unique_ptr<MouseObserver> m_observer;
    scoped_connection m_connection;
  };
  using FilteredMouseSignal = boost::signals2::signal<
    bool (QWidget& target, QMouseEvent& event), MouseCombiner>;

  mutable FilteredMouseSignal m_mouse_signal;
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
    if(event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease) {
      auto filter = m_mouse_signal(*static_cast<QWidget*>(watched),
        *static_cast<QMouseEvent*>(event));
      return filter;
    } else if(event->type() == QEvent::ChildAdded ||
        event->type() == QEvent::ChildPolished) {
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
    auto i = m_children.find(&child);
    if(i != m_children.end()) {
      return;
    }
    auto observer = std::make_unique<MouseObserver>(child);
    auto connection = observer->connect_filtered_mouse_signal(
      [=] (auto& target, auto& event) {
        return m_mouse_signal(target, event);
      });
    auto child_entry = Child(std::move(observer), std::move(connection));
    m_children.insert(i, std::pair(&child, std::move(child_entry)));
  }
};

MouseObserver::MouseObserver(QWidget& widget) {
  m_filter = find_extension<EventFilter>(widget);
  m_filter_connection = m_filter->m_mouse_signal.connect(
    [=] (auto& target, auto& event) {
      return m_mouse_signal(target, event).value_or(false);
    });
}

connection MouseObserver::connect_filtered_mouse_signal(
    const FilteredMouseSignal::slot_type& slot) const {
  return m_mouse_signal.connect(slot);
}

connection MouseObserver::connect_mouse_signal(const MouseSignal& slot) const {
  return connect_filtered_mouse_signal([=] (auto& target, auto& event) {
    slot(target, event);
    return false;
  });
}
