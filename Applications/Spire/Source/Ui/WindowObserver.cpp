#include "Spire/Ui/WindowObserver.hpp"
#include <QEvent>

using namespace boost::signals2;
using namespace Spire;

struct WindowObserver::EventFilter : QObject {
  mutable WindowSignal m_window_signal;
  QWidget* m_widget;
  QWidget* m_window;
  std::unique_ptr<WindowObserver> m_parent_observer;
  scoped_connection m_parent_connection;

  EventFilter(QWidget& widget)
      : m_widget(&widget),
        m_window(m_widget->window()) {
    m_widget->installEventFilter(this);
    m_widget->connect(
      m_widget, &QObject::destroyed, this, &EventFilter::on_widget_destroyed);
    observe_parent();
  }

  void set_window(QWidget* window) {
    if(m_window == window) {
      return;
    }
    m_window = window;
    m_window_signal(window);
  }

  void observe_parent() {
    m_parent_connection.disconnect();
    if(m_widget->parentWidget()) {
      m_parent_observer =
        std::make_unique<WindowObserver>(*m_widget->parentWidget());
      m_parent_connection = m_parent_observer->connect_window_signal(
        std::bind_front(&EventFilter::on_window, this));
    } else {
      m_parent_observer = nullptr;
    }
    set_window(m_widget->window());
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::ParentChange) {
      observe_parent();
    }
    return QObject::eventFilter(watched, event);
  }

  void on_widget_destroyed() {
    m_parent_connection.disconnect();
  }

  void on_window(QWidget* window) {
    set_window(window);
  }
};

WindowObserver::WindowObserver(QWidget& widget)
  : m_event_filter(std::make_unique<EventFilter>(widget)) {}

WindowObserver::~WindowObserver() = default;

QWidget* WindowObserver::get_window() const {
  return m_event_filter->m_window;
}

connection WindowObserver::connect_window_signal(
    const WindowSignal::slot_type& slot) const {
  return m_event_filter->m_window_signal.connect(slot);
}
