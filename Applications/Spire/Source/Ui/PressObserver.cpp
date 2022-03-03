#include "Spire/Ui/PressObserver.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWidget>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

struct PressObserver::PressEventFilter : QObject {
  mutable PressStartSignal m_press_start_signal;
  mutable PressEndSignal m_press_end_signal;
  bool m_is_mouse_down;
  bool m_is_key_down;

  PressEventFilter(const QWidget& widget)
      : m_is_mouse_down(false),
        m_is_key_down(false) {
    const_cast<QWidget&>(widget).installEventFilter(this);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::FocusOut) {
      auto& focus_event = *static_cast<QFocusEvent*>(event);
      if(focus_event.reason() != Qt::PopupFocusReason && m_is_key_down) {
        m_is_key_down = false;
        m_press_end_signal(Reason::KEYBOARD);
      }
    } else if(event->type() == QEvent::KeyPress) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      if(key_event.key() == Qt::Key_Space) {
        key_event.accept();
        if(!key_event.isAutoRepeat() && !m_is_key_down) {
          m_is_key_down = true;
          m_press_start_signal(Reason::KEYBOARD);
        }
      }
    } else if(event->type() == QEvent::KeyRelease) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      if(key_event.key() == Qt::Key_Space) {
        key_event.accept();
        if(!key_event.isAutoRepeat() && m_is_key_down) {
          m_is_key_down = false;
          m_press_end_signal(Reason::KEYBOARD);
        }
      }
    } else if(event->type() == QEvent::MouseButtonPress) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(mouse_event.button() == Qt::LeftButton && !m_is_mouse_down) {
        m_is_mouse_down = true;
        mouse_event.accept();
        m_press_start_signal(Reason::MOUSE);
      }
    } else if(event->type() == QEvent::MouseButtonRelease) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(mouse_event.button() == Qt::LeftButton && m_is_mouse_down) {
        m_is_mouse_down = false;
        mouse_event.accept();
        m_press_end_signal(Reason::MOUSE);
      }
    }
    return QObject::eventFilter(watched, event);
  }
};

PressObserver::PressObserver(const QWidget& widget)
  : m_filter(std::make_unique<PressEventFilter>(widget)) {}

PressObserver::~PressObserver() = default;

connection PressObserver::connect_press_start_signal(
    const PressStartSignal::slot_type& slot) const {
  return m_filter->m_press_start_signal.connect(slot);
}

connection PressObserver::connect_press_end_signal(
    const PressStartSignal::slot_type& slot) const {
  return m_filter->m_press_end_signal.connect(slot);
}
