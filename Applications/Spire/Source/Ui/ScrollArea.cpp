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
  : ScrollArea(false, parent) {}

ScrollArea::ScrollArea(bool is_dynamic, QWidget* parent)
    : QScrollArea(parent),
      m_is_dynamic(is_dynamic),
      m_is_wheel_disabled(false),
      m_horizontal_scrolling_error(0.0),
      m_vertical_scrolling_error(0.0),
      m_border_color(Qt::transparent),
      m_border_width(0) {
  setFrameStyle(QFrame::NoFrame);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_scroll_bar_style = new ScrollBarStyle(this);
  horizontalScrollBar()->setStyle(m_scroll_bar_style);
  verticalScrollBar()->setStyle(m_scroll_bar_style);
}

void ScrollArea::set_border_style(int width, const QColor& color) {
  m_border_color = color;
  m_border_width = width;
  if(verticalScrollBar()->width() == SCROLL_BAR_MIN_SIZE) {
    set_scroll_bar_style(SCROLL_BAR_MIN_SIZE);
  } else {
    set_scroll_bar_style(SCROLL_BAR_MAX_SIZE);
  }
}

void ScrollArea::setWidget(QWidget* widget) {
  if(m_is_dynamic) {
    widget->setMouseTracking(true);
    widget->setAttribute(Qt::WA_Hover);
    widget->installEventFilter(this);
  }
  for(auto& child : widget->children()) {
    if(auto c = qobject_cast<QWidget*>(child)) {
      c->installEventFilter(this);
    }
  }
  QScrollArea::setWidget(widget);
}

bool ScrollArea::eventFilter(QObject* watched, QEvent* event) {
  if(m_is_dynamic) {
    if(event->type() == QEvent::HoverMove) {
      auto e = static_cast<QHoverEvent*>(event);
      if(widget()->width() > width() && is_within_opposite_scroll_bar(
          verticalScrollBar(), e->pos().y(), widget()->height(), height()) &&
          !verticalScrollBar()->isVisible()) {
        set_scroll_bar_style(SCROLL_BAR_MAX_SIZE);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      } else if(widget()->height() > height() && is_within_opposite_scroll_bar(
          horizontalScrollBar(), e->pos().x(), widget()->width(), width()) &&
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
  }
  return QScrollArea::eventFilter(watched, event);
}

void ScrollArea::leaveEvent(QEvent* event) {
  if(m_is_dynamic) {
    hide_horizontal_scroll_bar();
    hide_vertical_scroll_bar();
  }
}

void ScrollArea::showEvent(QShowEvent* event) {
  if(m_is_dynamic) {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_horizontal_scroll_bar_timer.setInterval(SCROLL_BAR_HIDE_TIME_MS);
    connect(&m_horizontal_scroll_bar_timer, &QTimer::timeout, this,
      &ScrollArea::hide_horizontal_scroll_bar);
    m_vertical_scroll_bar_timer.setInterval(SCROLL_BAR_HIDE_TIME_MS);
    connect(&m_vertical_scroll_bar_timer, &QTimer::timeout, this,
      &ScrollArea::hide_vertical_scroll_bar);
    set_scroll_bar_style(SCROLL_BAR_MIN_SIZE);
  } else {
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    set_scroll_bar_style(SCROLL_BAR_MAX_SIZE);
  }
}

void ScrollArea::wheelEvent(QWheelEvent* event) {
  if(m_is_wheel_disabled) {
    return;
  }
  if(event->modifiers().testFlag(Qt::ShiftModifier)) {
    if(m_is_dynamic) {
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      m_horizontal_scroll_bar_timer.start();
    }
    update_scrollbar_position(horizontalScrollBar(), event->angleDelta().y(),
      m_horizontal_scrolling_error);
  } else {
    if(m_is_dynamic) {
      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      m_vertical_scroll_bar_timer.start();
    }
    update_scrollbar_position(verticalScrollBar(), event->angleDelta().y(),
      m_vertical_scrolling_error);
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
    int scroll_size, int widget_size) const {
  if(widget() == nullptr) {
    return false;
  }
  auto scroll_adjustment = [&] {
    if(scroll_size <= widget_size) {
      return 0;
    }
    return map_to(static_cast<double>(scroll_bar->value()),
      static_cast<double>(scroll_bar->minimum()),
      static_cast<double>(scroll_bar->maximum()), 0,
      scroll_size - widget_size);
  }();
  return pos - scroll_adjustment > widget_size -
    scale_width(SCROLL_BAR_MAX_SIZE);
}

void ScrollArea::update_scrollbar_position(QScrollBar* scroll_bar, int delta,
    double& scrolling_error) {
  auto adjusted_delta = delta / 2 + scrolling_error;
  scrolling_error = std::modf(adjusted_delta, &adjusted_delta);
  scroll_bar->setValue(scroll_bar->value() -
    static_cast<int>(adjusted_delta));
}

void ScrollArea::set_scroll_bar_style(int handle_size) {
  m_scroll_bar_style->set_horizontal_slider_height(scale_height(handle_size));
  m_scroll_bar_style->set_vertical_slider_width(scale_width(handle_size));
  update();
}
