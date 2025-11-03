#include "Spire/Ui/BreakoutBox.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QWheelEvent>
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;

BreakoutBox::BreakoutBox(QWidget& body, QWidget& source)
    : QWidget(source.window()),
      m_body(&body),
      m_source(&source) {
  enclose(*this, *m_body);
  setFocusProxy(m_body);
  move(parentWidget()->mapFromGlobal(m_source->mapToGlobal(QPoint(0, 0))));
  setMinimumSize(m_source->size());
  update_maximum_size();
  m_source->installEventFilter(this);
  parentWidget()->installEventFilter(this);
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
  if(watched == m_source) {
    if(event->type() == QEvent::Move) {
      move(parentWidget()->mapFromGlobal(m_source->mapToGlobal(QPoint(0, 0))));
      update_maximum_size();
    } else if(event->type() == QEvent::Resize) {
      auto& resize_event = *static_cast<QResizeEvent*>(event);
      setMinimumSize(resize_event.size());
      adjustSize();
    }
  } else if(watched == parentWidget() && event->type() == QEvent::Resize) {
    update_maximum_size();
  } else if(event->type() == QEvent::Wheel && isVisible()) {
    auto& wheel_event = *static_cast<QWheelEvent*>(event);
    auto parent = parentWidget();
    auto global_pos = wheel_event.globalPos();
    if(parent->rect().contains(parent->mapFromGlobal(global_pos)) &&
        !rect().contains(mapFromGlobal(global_pos))) {
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool BreakoutBox::focusNextPrevChild(bool next) {
  auto focus_reason = [&] {
    if(next) {
      return Qt::FocusReason::TabFocusReason;
    }
    return Qt::FocusReason::BacktabFocusReason;
  }();
  m_source->setFocus(focus_reason);
  return focus_next(*m_source, next);
}

void BreakoutBox::update_maximum_size() {
  auto parent = parentWidget();
  setMaximumWidth(parent->width() - x());
  setMaximumHeight(parent->height() - y());
}
