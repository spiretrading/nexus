#include "Spire/Ui/ScrollBarStyle.hpp"
#include <QPainter>
#include <QStyleOption>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

namespace {
  auto MINIMUM_HORIZONAL_HANDLE_WIDTH() {
    static auto width = scale_width(60);
    return width;
  }

  auto MINIMUM_VERTICAL_HANDLE_HEIGHT() {
    static auto height = scale_height(60);
    return height;
  }
}

// TODO: constructor parameters; lifetime?
ScrollBarStyle::ScrollBarStyle(QStyle* style = nullptr,
    QWidget* parent = nullptr)
    : QProxyStyle(style),
      m_horizontal_handle_height(scale_height(13)),
      m_vertical_handle_width(scale_width(13)) {
  //setParent(parent);
}

void ScrollBarStyle::drawComplexControl(QStyle::ComplexControl control,
    const QStyleOptionComplex* option, QPainter* painter,
    const QWidget *widget) const {
  if(control == CC_ScrollBar) {
    painter->fillRect(option->rect, Qt::white);
    auto slider_option = QStyleOptionSlider();
    slider_option.rect = subControlRect(CC_ScrollBar, &slider_option,
      SC_ScrollBarSlider, widget);
    drawControl(CE_ScrollBarSlider, &slider_option, painter, widget);
    return;
  }
  QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void ScrollBarStyle::drawControl(QStyle::ControlElement element,
    const QStyleOption* option, QPainter* painter,
    const QWidget *widget) const {
  if(element == CE_ScrollBarSlider) {
    painter->fillRect(option->rect, QColor("#C8C8C8"));
    return;
  }
  QProxyStyle::drawControl(element, option, painter, widget);
}

int ScrollBarStyle::pixelMetric(PixelMetric metric,
    const QStyleOption* option, const QWidget* widget) const {
  if(metric == PM_ScrollBarExtent) {
    auto scroll_bar = qobject_cast<const QScrollBar*>(widget);
    if(scroll_bar->orientation() == Qt::Horizontal) {
      return m_horizontal_handle_height;
    } else {
      return m_vertical_handle_width;
    }
  }
  return QProxyStyle::pixelMetric(metric, option, widget);
}

int ScrollBarStyle::styleHint(QStyle::StyleHint hint,
    const QStyleOption* option, const QWidget* widget,
    QStyleHintReturn* return_data) const {
  if(hint == QStyle::SH_ScrollBar_ContextMenu) {
    return 0;
  }
  return QProxyStyle::styleHint(hint, option, widget, return_data);
}

QRect ScrollBarStyle::subControlRect(ComplexControl control,
    const QStyleOptionComplex* option, SubControl sub_control,
    const QWidget* widget) const {
  switch(sub_control) {
    case SC_ScrollBarAddPage:
      {
        auto scroll_bar = qobject_cast<const QScrollBar*>(widget);
        if(scroll_bar->orientation() == Qt::Horizontal) {
          auto slider_right_pos = get_horizontal_slider_position(
            scroll_bar) + MINIMUM_HORIZONAL_HANDLE_WIDTH();
          return {slider_right_pos, 0,
            scroll_bar->width() - slider_right_pos,
            m_horizontal_handle_height};
        } else {
          auto slider_bottom_pos = get_vertical_slider_position(
            scroll_bar) + MINIMUM_VERTICAL_HANDLE_HEIGHT();
          return {0, slider_bottom_pos, m_vertical_handle_width,
            scroll_bar->height() - slider_bottom_pos};
        }
      }
    case SC_ScrollBarSubPage:
      {
        auto scroll_bar = qobject_cast<const QScrollBar*>(widget);
        if(scroll_bar->orientation() == Qt::Horizontal) {
          auto slider_pos = get_horizontal_slider_position(scroll_bar);
          return {0, 0, slider_pos,
            slider_pos + MINIMUM_HORIZONAL_HANDLE_WIDTH()};
        } else {
          return {0, 0, m_vertical_handle_width,
            get_vertical_slider_position(scroll_bar)};
        }
      }
    case SC_ScrollBarGroove:
      return widget->rect();
    case SC_ScrollBarSlider:
      {
        auto scroll_bar = qobject_cast<const QScrollBar*>(widget);
        if(scroll_bar->orientation() == Qt::Horizontal) {
          return QRect(get_horizontal_slider_position(scroll_bar), 0,
            MINIMUM_HORIZONAL_HANDLE_WIDTH(), m_horizontal_handle_height);
        } else {
          return QRect(0, get_vertical_slider_position(scroll_bar),
            m_vertical_handle_width, MINIMUM_VERTICAL_HANDLE_HEIGHT());
        }
      }
    case SC_ScrollBarAddLine:
    case SC_ScrollBarFirst:
    case SC_ScrollBarLast:
    case SC_ScrollBarSubLine:
      return {};
  }
  return QProxyStyle::subControlRect(control, option, sub_control, widget);
}

void ScrollBarStyle::set_horizontal_slider_height(int height) {
  m_horizontal_handle_height = height;
}

void ScrollBarStyle::set_vertical_slider_width(int width) {
  m_vertical_handle_width = width;
}

int ScrollBarStyle::get_horizontal_slider_position(
    const QScrollBar* scroll_bar) const {
  return sliderPositionFromValue(scroll_bar->minimum(),
    scroll_bar->maximum(), scroll_bar->value(),
    scroll_bar->width() - MINIMUM_HORIZONAL_HANDLE_WIDTH());
}

int ScrollBarStyle::get_vertical_slider_position(
    const QScrollBar* scroll_bar) const {
  return sliderPositionFromValue(scroll_bar->minimum(),
    scroll_bar->maximum(), scroll_bar->value(),
    scroll_bar->height() - MINIMUM_VERTICAL_HANDLE_HEIGHT());
}
