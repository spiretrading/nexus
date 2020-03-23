#include "Spire/Ui/ScrollArea.hpp"
#include <QMouseEvent>
#include <QScrollBar>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

namespace {
  const auto MINIMUM_TABLE_WIDTH = 750;
  const auto SCROLL_BAR_FADE_TIME_MS = 500;
  const auto SCROLL_BAR_MAX_SIZE = 13;
  const auto SCROLL_BAR_MIN_SIZE = 6;
}

ScrollArea::ScrollArea(QWidget* parent)
    : QScrollArea(parent) {
  setMouseTracking(true);
  setAttribute(Qt::WA_Hover);
  horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_h_scroll_bar_timer.setInterval(SCROLL_BAR_FADE_TIME_MS);
  connect(&m_h_scroll_bar_timer, &QTimer::timeout, this,
    &ScrollArea::fade_out_horizontal_scroll_bar);
  m_v_scroll_bar_timer.setInterval(SCROLL_BAR_FADE_TIME_MS);
  connect(&m_v_scroll_bar_timer, &QTimer::timeout, this,
    &ScrollArea::fade_out_vertical_scroll_bar);
}

void ScrollArea::leaveEvent(QEvent* event) {
  if(!m_h_scroll_bar_timer.isActive()) {
  fade_out_horizontal_scroll_bar();
  }
  if(!m_v_scroll_bar_timer.isActive()) {
  fade_out_vertical_scroll_bar();
  }
}

void ScrollArea::mouseMoveEvent(QMouseEvent* event) {
  if(is_within_horizontal_scroll_bar(event->pos()) &&
      !verticalScrollBar()->isVisible()) {
    set_scroll_bar_style(SCROLL_BAR_MAX_SIZE);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  } else if(is_within_vertical_scroll_bar(event->pos()) &&
      !horizontalScrollBar()->isSliderDown()) {
    set_scroll_bar_style(SCROLL_BAR_MAX_SIZE);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  } else {
    set_scroll_bar_style(SCROLL_BAR_MIN_SIZE);
    if(!m_v_scroll_bar_timer.isActive() &&
        verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff &&
        !verticalScrollBar()->isSliderDown()) {
      fade_out_vertical_scroll_bar();
    }
    if(!m_h_scroll_bar_timer.isActive() &&
        horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff &&
        !horizontalScrollBar()->isSliderDown()) {
      fade_out_horizontal_scroll_bar();
    }
  }
}

void ScrollArea::fade_out_horizontal_scroll_bar() {
  m_h_scroll_bar_timer.stop();
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void ScrollArea::fade_out_vertical_scroll_bar() {
  m_v_scroll_bar_timer.stop();
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

bool ScrollArea::is_within_horizontal_scroll_bar(
    const QPoint& pos) {
  return pos.y() > height() - scale_height(SCROLL_BAR_MAX_SIZE);
}

bool ScrollArea::is_within_vertical_scroll_bar(
    const QPoint& pos) {
  return pos.x() > width() - scale_width(SCROLL_BAR_MAX_SIZE);
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
