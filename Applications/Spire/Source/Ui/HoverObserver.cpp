#include "Spire/Ui/HoverObserver.hpp"
#include <QApplication>
#include <QChildEvent>
#include "Spire/Spire/ExtensionCache.hpp"
#include "Spire/Ui/GlobalPositionObserver.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  HoverObserver::State get_state(const QWidget& widget, QPoint position) {
    if(!widget.isEnabled()) {
      return HoverObserver::State::NONE;
    }
    auto cursor = QCursor::pos();
    if(QRect(position, widget.size()).contains(cursor)) {
      auto hovered_widget = qApp->widgetAt(cursor);
      if(hovered_widget == &widget) {
        return HoverObserver::State::MOUSE_IN;
      } else if(widget.isAncestorOf(hovered_widget)) {
        return HoverObserver::State::MOUSE_OVER;
      }
    }
    return HoverObserver::State::NONE;
  }
}

struct HoverObserver::EventFilter : QObject {
  struct Child {
    std::unique_ptr<HoverObserver> m_observer;
    scoped_connection m_state_connection;
  };
  struct Observers {
    GlobalPositionObserver m_position_observer;
    std::unordered_map<QObject*, Child> m_children_observers;

    Observers(QWidget& widget, std::function<void (const QPoint&)> on_position)
        : m_position_observer(widget) {
      m_position_observer.connect_position_signal(std::move(on_position));
    }
  };
  mutable StateSignal m_state_signal;
  QWidget* m_widget;
  State m_state;
  std::unique_ptr<Observers> m_observers;

  EventFilter(QWidget& widget)
      : m_widget(&widget),
        m_state(State::NONE) {
    widget.connect(
      &widget, &QObject::destroyed, this, &EventFilter::destroy_observers);
    widget.installEventFilter(this);
    if(widget.isEnabled() && widget.isVisible()) {
      initialize_observers();
    }
  }

  void initialize_observers() {
    m_observers = std::make_unique<Observers>(
      *m_widget, std::bind_front(&EventFilter::on_position, this));
    set_state(
      ::get_state(*m_widget, m_observers->m_position_observer.get_position()));
    for(auto child : m_widget->children()) {
      if(child && child->isWidgetType()) {
        add(static_cast<QWidget&>(*child));
      }
    }
    connect(
      qApp, &QApplication::focusChanged, this, &EventFilter::on_focus_changed);
  }

  void destroy_observers() {
    if(m_observers) {
      m_observers = nullptr;
      disconnect(qApp, &QApplication::focusChanged, this, nullptr);
      set_state(State::NONE);
    }
  }

  Observers& get_observers() {
    if(!m_observers) {
      initialize_observers();
    }
    return *m_observers;
  }

  void set_state(State state) {
    if(state == m_state) {
      return;
    }
    m_state = state;
    m_state_signal(state);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(!m_widget->isEnabled() || !m_widget->isVisible()) {
      destroy_observers();
    } else if(event->type() == QEvent::Enter ||
        event->type() == QEvent::EnabledChange ||
        event->type() == QEvent::MouseMove) {
      set_state(::get_state(
        *m_widget, get_observers().m_position_observer.get_position()));
    } else if(event->type() == QEvent::Leave) {
      set_state(State::NONE);
    } else if(event->type() == QEvent::ChildAdded) {
      auto& child = *static_cast<QChildEvent&>(*event).child();
      if(child.isWidgetType()) {
        add(static_cast<QWidget&>(child));
      }
    } else if(event->type() == QEvent::ChildRemoved) {
      auto& child_event = static_cast<QChildEvent&>(*event);
      if(m_observers &&
          m_observers->m_children_observers.contains(child_event.child())) {
        get_observers().m_children_observers.erase(child_event.child());
        set_state(::get_state(
          *m_widget, get_observers().m_position_observer.get_position()));
      }
    }
    return QObject::eventFilter(watched, event);
  }

  void add(QWidget& child) {
    auto observer = std::make_unique<HoverObserver>(child);
    auto connection = observer->connect_state_signal(
      std::bind_front(&EventFilter::on_hover, this));
    auto child_entry = Child(std::move(observer), std::move(connection));
    get_observers().m_children_observers.insert(
      std::pair(&child, std::move(child_entry)));
    set_state(::get_state(
      *m_widget, get_observers().m_position_observer.get_position()));
  }

  void on_hover(State state) {
    set_state(::get_state(
      *m_widget, get_observers().m_position_observer.get_position()));
  }

  void on_position(const QPoint& position) {
    if(m_state != HoverObserver::State::NONE ||
        QApplication::mouseButtons() == Qt::NoButton) {
      set_state(::get_state(*m_widget, position));
    }
  }

  void on_focus_changed(QWidget* old, QWidget* now) {
    set_state(::get_state(
      *m_widget, get_observers().m_position_observer.get_position()));
  }
};

HoverObserver::HoverObserver(QWidget& widget) {
  m_filter = find_extension<EventFilter>(widget);
  m_filter_connection = m_filter->m_state_signal.connect(m_state_signal);
}

HoverObserver::State HoverObserver::get_state() const {
  return m_filter->m_state;
}

connection HoverObserver::connect_state_signal(
    const StateSignal::slot_type& slot) const {
  return m_state_signal.connect(slot);
}

bool Spire::is_set(HoverObserver::State left, HoverObserver::State right) {
  return static_cast<HoverObserver::State>(
    static_cast<std::underlying_type_t<HoverObserver::State>>(left) &
    static_cast<std::underlying_type_t<HoverObserver::State>>(right)) !=
    HoverObserver::State::NONE;
}
