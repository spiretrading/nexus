#include "Spire/Ui/BreakoutBox.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QWheelEvent>
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;

namespace {
  template<bool (QWidget::* method)(bool)>
  struct FocusNext {
    friend bool focus_next(QWidget& widget, bool next) {
      return (widget.*method)(next);
    }
  };

  template struct FocusNext<&QWidget::focusNextPrevChild>;

  bool focus_next(QWidget& widget, bool next);
}

BreakoutBox::BreakoutBox(QWidget& body, QWidget& source)
    : QWidget(source.window()),
      m_body(&body),
      m_source(&source) {
  auto pos = m_body->mapToGlobal(QPoint(0, 0));
  enclose(*this, *m_body);
  setFocusProxy(m_body);
  setMinimumSize(body.size());
  resize(m_body->sizeHint());
  move(parentWidget()->mapFromGlobal(pos));
  m_source->installEventFilter(this);
  qApp->installEventFilter(this);
}

const QWidget& BreakoutBox::get_body() const {
  return *m_body;
}

QWidget& BreakoutBox::get_body() {
  return *m_body;
}

bool BreakoutBox::event(QEvent* event) {
  if(event->type() == QEvent::LayoutRequest) {
    adjustSize();
    return true;
  }
  return QWidget::event(event);
}

bool BreakoutBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_source && event->type() == QEvent::Resize) {
    auto& resize_event = *static_cast<QResizeEvent*>(event);
    setMinimumSize(resize_event.size());
    adjustSize();
  } else if(event->type() == QEvent::Wheel && isVisible()) {
    auto& wheel_event = *static_cast<QWheelEvent*>(event);
    auto parent_window = window();
    if(parent_window->rect().contains(
        parent_window->mapFromGlobal(wheel_event.globalPos()))) {
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool BreakoutBox::focusNextPrevChild(bool next) {
  return focus_next(*m_source, next);
}
