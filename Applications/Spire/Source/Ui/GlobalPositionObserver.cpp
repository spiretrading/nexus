#include "Spire/Ui/GlobalPositionObserver.hpp"
#include <QEvent>
#include <QPointer>
#include "Spire/Spire/ExtensionCache.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  QPoint get_position(const QWidget& widget) {
    return widget.mapToGlobal(QPoint(0, 0));
  }
}

struct GlobalPositionObserver::EventFilter : QObject {
  mutable PositionSignal m_position_signal;
  QPointer<QWidget> m_widget;
  QPoint m_position;
  std::shared_ptr<EventFilter> m_parent_filter;
  scoped_connection m_parent_position_connection;

  EventFilter(QWidget& widget)
      : m_widget(&widget),
        m_position(::get_position(*m_widget)) {
    m_widget->installEventFilter(this);
    connect(
      m_widget, &QObject::destroyed, this, &EventFilter::on_widget_destroyed);
    observe_parent();
  }

  void set_position(QPoint position) {
    if(position == m_position) {
      return;
    }
    m_position = position;
    m_position_signal(position);
  }

  void observe_parent() {
    m_parent_position_connection.disconnect();
    if(m_widget->parentWidget()) {
      m_parent_filter = find_extension<EventFilter>(*m_widget->parentWidget());
      m_parent_position_connection = m_parent_filter->m_position_signal.connect(
        std::bind_front(&EventFilter::on_position, this));
    } else {
      m_parent_filter = nullptr;
    }
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::Move ||
        event->type() == QEvent::LayoutRequest) {
      set_position(::get_position(*m_widget));
    } else if(event->type() == QEvent::ParentChange) {
      observe_parent();
    }
    return QObject::eventFilter(watched, event);
  }

  void on_widget_destroyed() {
    m_parent_position_connection.disconnect();
    m_parent_filter = nullptr;
  }

  void on_position(const QPoint& position) {
    if(m_widget) {
      set_position(::get_position(*m_widget));
    }
  }
};

GlobalPositionObserver::GlobalPositionObserver(QWidget& widget) {
  m_filter = find_extension<EventFilter>(widget);
  m_position_connection =
    m_filter->m_position_signal.connect(m_position_signal);
}

QPoint GlobalPositionObserver::get_position() const {
  return m_filter->m_position;
}

connection GlobalPositionObserver::connect_position_signal(
    const PositionSignal::slot_type& slot) const {
  return m_position_signal.connect(slot);
}
