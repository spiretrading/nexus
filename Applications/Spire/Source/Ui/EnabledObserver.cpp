#include "Spire/Ui/EnabledObserver.hpp"
#include <QEvent>
#include <QPointer>
#include <QWidget>
#include "Spire/Spire/ExtensionCache.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

struct EnabledObserver::EventFilter : QObject {
  mutable EnabledSignal m_enabled_signal;
  QPointer<QWidget> m_widget;
  bool m_is_enabled;

  EventFilter(QWidget& widget)
      : m_widget(&widget),
        m_is_enabled(widget.isEnabled()) {
    m_widget->installEventFilter(this);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::EnabledChange ||
        event->type() == QEvent::ParentChange) {
      update();
    }
    return QObject::eventFilter(watched, event);
  }

  void update() {
    if(m_is_enabled == m_widget->isEnabled()) {
      return;
    }
    m_is_enabled = m_widget->isEnabled();
    if(m_is_enabled) {
      m_enabled_signal(true);
    } else {
      m_enabled_signal(false);
    }
  }
};

EnabledObserver::EnabledObserver(QWidget& widget) {
  m_filter = find_extension<EventFilter>(widget);
  m_filter_connection = m_filter->m_enabled_signal.connect(m_enabled_signal);
}

connection EnabledObserver::connect_enabled_signal(
    const EnabledSignal::slot_type& slot) const {
  return m_enabled_signal.connect(slot);
}
