#include "Spire/Ui/BreakoutBox.hpp"
#include <QApplication>
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
  update_size_constraints();
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
  }
  return QWidget::event(event);
}

bool BreakoutBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_source) {
    if(event->type() == QEvent::Move) {
      move(parentWidget()->mapFromGlobal(m_source->mapToGlobal(QPoint(0, 0))));
      update_size();
    } else if(event->type() == QEvent::Resize) {
      update_size();
    }
  } else if(watched == parentWidget() && event->type() == QEvent::Resize) {
    update_size();
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

void BreakoutBox::update_size_constraints() {
  auto parent = parentWidget();
  auto maximum_width = parent->width() - x();
  auto maximum_height = parent->height() - y();
  setMaximumSize(maximum_width, maximum_height);
  auto minimum_width = m_source->width();
  if(maximum_width <= minimum_width) {
    minimum_width = maximum_width;
  }
  auto minimum_height = m_source->height();
  if(maximum_height <= minimum_height) {
    minimum_height = maximum_height;
  }
  setMinimumSize(minimum_width, minimum_height);
}

void BreakoutBox::update_size() {
  update_size_constraints();
  invalidate_descendant_layouts(*this);
  adjustSize();
}
