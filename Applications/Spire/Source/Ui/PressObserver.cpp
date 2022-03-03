#include "Spire/Ui/PressObserver.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWidget>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

struct PressObserver::PressEventFilter : QObject {
  mutable PressSignal m_press_signal;
  bool m_is_down;

  PressEventFilter(const QWidget& widget)
      : m_is_down(false) {
    const_cast<QWidget&>(widget).installEventFilter(this);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::KeyPress) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      if(key_event.key() == Qt::Key_Space) {
        if(!key_event.isAutoRepeat()) {
          m_is_down = true;
        }
        key_event.accept();
        m_press_signal();
        return true;
      }
    } else if(event->type() == QEvent::KeyRelease) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      if(key_event.key() == Qt::Key_Space) {
        if(!key_event.isAutoRepeat() && m_is_down) {
          m_is_down = false;
        }
        key_event.accept();
        return true;
      }
    } else if(event->type() == QEvent::MouseButtonPress) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(mouse_event.button() == Qt::LeftButton &&
          static_cast<QWidget*>(watched)->rect().contains(mouse_event.pos())) {
        m_is_down = true;
        mouse_event.accept();
        m_press_signal();
        return true;
      }
    } else if(event->type() == QEvent::MouseButtonRelease) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(mouse_event.button() == Qt::LeftButton) {
        if(static_cast<QWidget*>(watched)->rect().contains(mouse_event.pos())) {
          mouse_event.accept();
          if(m_is_down) {
            m_is_down = false;
          }
          return true;
        }
      }
    }
    return QObject::eventFilter(watched, event);
  }
};

PressObserver::PressObserver(const QWidget& widget)
  : m_filter(std::make_unique<PressEventFilter>(widget)) {}

PressObserver::~PressObserver() = default;

connection PressObserver::connect_press_signal(
    const PressSignal::slot_type& slot) const {
  return m_filter->m_press_signal.connect(slot);
}
