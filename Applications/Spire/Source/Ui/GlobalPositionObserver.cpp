#include "Spire/Ui/GlobalPositionObserver.hpp"
#include <QEvent>

using namespace boost::signals2;
using namespace Spire;

struct GlobalPositionObserver::EventFilter : QObject {
  mutable PositionSignal m_position_signal;
  QWidget* m_widget;
  QPoint m_position;
  std::unique_ptr<GlobalPositionObserver> m_parent_observer;

  EventFilter(QWidget& widget)
      : m_widget(&widget),
        m_position(m_widget->mapToGlobal(m_widget->pos())) {
    m_widget->installEventFilter(this);
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
    if(m_widget->parentWidget()) {
      m_parent_observer =
        std::make_unique<GlobalPositionObserver>(*m_widget->parentWidget());
      m_parent_observer->connect_position_signal(
        std::bind_front(&EventFilter::on_position, this));
    }
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::Move) {
      set_position(m_widget->mapToGlobal(m_widget->pos()));
    } else if(event->type() == QEvent::ParentChange) {
      observe_parent();
    }
    return QObject::eventFilter(watched, event);
  }

  void on_position(const QPoint& position) {
    set_position(m_widget->mapToGlobal(m_widget->pos()));
  }
};

GlobalPositionObserver::GlobalPositionObserver(QWidget& widget)
  : m_event_filter(std::make_unique<EventFilter>(widget)) {}

GlobalPositionObserver::~GlobalPositionObserver() = default;

QPoint GlobalPositionObserver::get_position() const {
  return m_event_filter->m_position;
}

connection GlobalPositionObserver::connect_position_signal(
    const PositionSignal::slot_type& slot) const {
  return m_event_filter->m_position_signal.connect(slot);
}
