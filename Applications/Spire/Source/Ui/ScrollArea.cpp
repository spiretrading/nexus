#include "Spire/Ui/ScrollArea.hpp"
#include <QHoverEvent>
#include <QScrollBar>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Spire;

namespace {
  const auto MINIMUM_TABLE_WIDTH = 750;
  const auto SCROLL_BAR_FADE_TIME_MS = 500;
  const auto SCROLL_BAR_MAX_SIZE = 13;
  const auto SCROLL_BAR_MIN_SIZE = 6;
}

ScrollArea::ScrollArea(QWidget* parent)
    : QScrollArea(parent) {
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

void ScrollArea::setWidget(QWidget* widget) {
  widget->setMouseTracking(true);
  widget->setAttribute(Qt::WA_Hover);
  widget->installEventFilter(this);
  QScrollArea::setWidget(widget);
}

bool ScrollArea::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::HoverMove) {
    auto e = static_cast<QHoverEvent*>(event);
    if(is_within_horizontal_scroll_bar(e->pos().y()) &&
        !verticalScrollBar()->isVisible()) {
      set_scroll_bar_style(SCROLL_BAR_MAX_SIZE);
      setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else if(is_within_vertical_scroll_bar(e->pos().x()) &&
        !horizontalScrollBar()->isSliderDown()) {
      set_scroll_bar_style(SCROLL_BAR_MAX_SIZE);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
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
  } else if(event->type() == QEvent::HoverLeave) {
    if(!m_h_scroll_bar_timer.isActive() &&
        horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
      fade_out_horizontal_scroll_bar();
    }
    if(!m_v_scroll_bar_timer.isActive() &&
        verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
      fade_out_vertical_scroll_bar();
    }
  }
  return false;
}

void ScrollArea::wheelEvent(QWheelEvent* event) {
  if(event->modifiers() & Qt::ShiftModifier) {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() -
      (event->delta() / 2));
    m_h_scroll_bar_timer.start();
  } else if(!event->modifiers().testFlag(Qt::ShiftModifier)) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    verticalScrollBar()->setValue(verticalScrollBar()->value() -
      (event->delta() / 2));
    m_v_scroll_bar_timer.start();
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

bool ScrollArea::is_within_horizontal_scroll_bar(int pos_y) {
  if(widget() == nullptr) {
    return false;
  }
  auto bar = verticalScrollBar();
  auto scroll_adj = map_to(static_cast<double>(bar->value()),
    static_cast<double>(bar->minimum()), static_cast<double>(bar->maximum()),
    0, widget()->height() - height());
  return pos_y - scroll_adj > height() - scale_height(SCROLL_BAR_MAX_SIZE);
}

bool ScrollArea::is_within_vertical_scroll_bar(int pos_x) {
  if(widget() == nullptr) {
    return false;
  }
  auto bar = horizontalScrollBar();
  auto scroll_adj = map_to(static_cast<double>(bar->value()),
    static_cast<double>(bar->minimum()), static_cast<double>(bar->maximum()),
    0, widget()->width() - width());
  return pos_x - scroll_adj > width() - scale_width(SCROLL_BAR_MAX_SIZE);
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
