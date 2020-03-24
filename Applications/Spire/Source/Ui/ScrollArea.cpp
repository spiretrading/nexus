#include "Spire/Ui/ScrollArea.hpp"
#include <QHoverEvent>
#include <QScrollBar>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Spire;

namespace {
  const auto MINIMUM_TABLE_WIDTH = 750;
  const auto SCROLL_BAR_HIDE_TIME_MS = 500;
  const auto SCROLL_BAR_MAX_SIZE = 13;
  const auto SCROLL_BAR_MIN_SIZE = 6;
}

ScrollArea::ScrollArea(QWidget* parent)
    : QScrollArea(parent) {
  setMouseTracking(true);
  horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_horizontal_scroll_bar_timer.setInterval(SCROLL_BAR_HIDE_TIME_MS);
  connect(&m_horizontal_scroll_bar_timer, &QTimer::timeout, this,
    &ScrollArea::hide_horizontal_scroll_bar);
  m_vertical_scroll_bar_timer.setInterval(SCROLL_BAR_HIDE_TIME_MS);
  connect(&m_vertical_scroll_bar_timer, &QTimer::timeout, this,
    &ScrollArea::hide_vertical_scroll_bar);
}

void ScrollArea::setWidget(QWidget* widget) {
  widget->setMouseTracking(true);
  widget->setAttribute(Qt::WA_Hover);
  widget->installEventFilter(this);
  QScrollArea::setWidget(widget);
}

bool ScrollArea::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::HoverMove) {
    auto e = static_cast<QHoverEvent*>(event);
    if(is_within_opposite_scroll_bar(verticalScrollBar(), e->pos().y(),
        widget()->height(), height()) && !verticalScrollBar()->isVisible()) {
      set_scroll_bar_style(SCROLL_BAR_MAX_SIZE);
      setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else if(is_within_opposite_scroll_bar(horizontalScrollBar(),
        e->pos().x(), widget()->width(), width()) &&
        !horizontalScrollBar()->isSliderDown()) {
      set_scroll_bar_style(SCROLL_BAR_MAX_SIZE);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
      set_scroll_bar_style(SCROLL_BAR_MIN_SIZE);
      if(!m_vertical_scroll_bar_timer.isActive() &&
          verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff &&
          !verticalScrollBar()->isSliderDown()) {
        hide_vertical_scroll_bar();
      }
      if(!m_horizontal_scroll_bar_timer.isActive() &&
          horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff &&
          !horizontalScrollBar()->isSliderDown()) {
        hide_horizontal_scroll_bar();
      }
    }
  } else if(event->type() == QEvent::HoverLeave) {
    if(!m_horizontal_scroll_bar_timer.isActive() &&
        horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
      hide_horizontal_scroll_bar();
    }
    if(!m_vertical_scroll_bar_timer.isActive() &&
        verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
      hide_vertical_scroll_bar();
    }
  }
  return QScrollArea::eventFilter(watched, event);
}

void ScrollArea::leaveEvent(QEvent* event) {
  hide_horizontal_scroll_bar();
  hide_vertical_scroll_bar();
}

void ScrollArea::wheelEvent(QWheelEvent* event) {
  if(event->modifiers().testFlag(Qt::ShiftModifier)) {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() -
      (event->delta() / 2));
    m_horizontal_scroll_bar_timer.start();
  } else {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    verticalScrollBar()->setValue(verticalScrollBar()->value() -
      (event->delta() / 2));
    m_vertical_scroll_bar_timer.start();
  }
}

void ScrollArea::hide_horizontal_scroll_bar() {
  m_horizontal_scroll_bar_timer.stop();
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void ScrollArea::hide_vertical_scroll_bar() {
  m_vertical_scroll_bar_timer.stop();
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

bool ScrollArea::is_within_opposite_scroll_bar(QScrollBar* scroll_bar, int pos,
    int scroll_size, int widget_size) {
  if(widget() == nullptr) {
    return false;
  }
  auto scroll_adjustment = map_to(static_cast<double>(scroll_bar->value()),
    static_cast<double>(scroll_bar->minimum()),
    static_cast<double>(scroll_bar->maximum()), 0, scroll_size - widget_size);
  return pos - scroll_adjustment > widget_size -
    scale_width(SCROLL_BAR_MAX_SIZE);
}

void ScrollArea::set_scroll_bar_style(int handle_size) {
  setStyleSheet(QString(R"(
    QWidget {
      background-color: #FFFFFF;
      border: none;
    }

    QScrollBar::horizontal {
      height: %1px;
    }

    QScrollBar::vertical {
      width: %2px;
    }

    QScrollBar::handle {
      background-color: #C8C8C8;
    }

    QScrollBar::handle:horizontal {
      min-width: %3px;
    }

    QScrollBar::handle:vertical {
      min-height: %4px;
    }

    QScrollBar::add-line, QScrollBar::sub-line,
    QScrollBar::add-page, QScrollBar::sub-page {
      background: none;
      border: none;
      height: 0px;
      width: 0px;
    })").arg(scale_height(handle_size)).arg(scale_width(handle_size))
        .arg(scale_width(60)).arg(scale_height(60)));
}
