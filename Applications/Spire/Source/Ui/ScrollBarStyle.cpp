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

ScrollBarStyle::ScrollBarStyle(QWidget* parent)
    : m_horizontal_handle_height(scale_height(13)),
      m_vertical_handle_width(scale_width(13)) {
  setParent(parent);
}

void ScrollBarStyle::drawComplexControl(QStyle::ComplexControl control,
    const QStyleOptionComplex* option, QPainter* painter,
    const QWidget *widget) const {
  if(control == QStyle::CC_ScrollBar) {
    painter->fillRect(option->rect, Qt::white);
    auto slider_option = QStyleOptionSlider();
    slider_option.rect = subControlRect(QStyle::CC_ScrollBar, &slider_option,
      QStyle::SC_ScrollBarSlider, widget);
    drawControl(QStyle::CE_ScrollBarSlider, &slider_option, painter, widget);
    return;
  }
  QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void ScrollBarStyle::drawControl(QStyle::ControlElement element,
    const QStyleOption* option, QPainter* painter,
    const QWidget *widget) const {
  if(element == QStyle::CE_ScrollBarSlider) {
    painter->fillRect(option->rect, QColor("#C8C8C8"));
    return;
  }
  QProxyStyle::drawControl(element, option, painter, widget);
}

int ScrollBarStyle::pixelMetric(PixelMetric metric,
    const QStyleOption* option, const QWidget* widget) const {
  if(metric == QStyle::PM_ScrollBarExtent) {
    if(auto scroll_bar = qobject_cast<const QScrollBar*>(widget); scroll_bar) {
      if(scroll_bar->orientation() == Qt::Horizontal) {
        return m_horizontal_handle_height;
      } else {
        return m_vertical_handle_width;
      }
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
  if(control == QStyle::CC_ScrollBar) {
    if(auto scroll_bar = qobject_cast<const QScrollBar*>(widget); scroll_bar) {
      switch(sub_control) {
        case QStyle::SC_ScrollBarAddPage:
          if(scroll_bar->orientation() == Qt::Horizontal) {
            auto slider_right_pos = get_horizontal_slider_position(
              scroll_bar) + get_handle_size(scroll_bar);
            return {slider_right_pos, 0,
              scroll_bar->width() - slider_right_pos,
              m_horizontal_handle_height};
          } else {
            auto slider_bottom_pos = get_vertical_slider_position(
              scroll_bar) + get_handle_size(scroll_bar);
            return {0, slider_bottom_pos, m_vertical_handle_width,
              scroll_bar->height() - slider_bottom_pos};
          }
        case QStyle::SC_ScrollBarSubPage:
          if(scroll_bar->orientation() == Qt::Horizontal) {
            return {0, 0, get_horizontal_slider_position(scroll_bar),
              m_horizontal_handle_height};
          }
          return {0, 0, m_vertical_handle_width,
            get_vertical_slider_position(scroll_bar)};
        case QStyle::SC_ScrollBarGroove:
          return widget->rect();
        case QStyle::SC_ScrollBarSlider:
          if(scroll_bar->orientation() == Qt::Horizontal) {
            return {get_horizontal_slider_position(scroll_bar), 0,
              get_handle_size(scroll_bar), scroll_bar->height()};
          }
          return {0, get_vertical_slider_position(scroll_bar),
            scroll_bar->width(), get_handle_size(scroll_bar)};
        case QStyle::SC_ScrollBarAddLine:
        case QStyle::SC_ScrollBarFirst:
        case QStyle::SC_ScrollBarLast:
        case QStyle::SC_ScrollBarSubLine:
          return {};
      }
    }
  }
  return QProxyStyle::subControlRect(control, option, sub_control, widget);
}

void ScrollBarStyle::set_horizontal_scroll_bar_height(int height) {
  m_horizontal_handle_height = height;
}

void ScrollBarStyle::set_vertical_scroll_bar_width(int width) {
  m_vertical_handle_width = width;
}

int ScrollBarStyle::get_handle_size(const QScrollBar* scroll_bar) const {
  if(scroll_bar->orientation() == Qt::Horizontal) {
    auto slider_size = (scroll_bar->pageStep() * scroll_bar->width()) /
      (scroll_bar->maximum() - scroll_bar->minimum() + scroll_bar->pageStep());
    return std::min(std::max(MINIMUM_HORIZONAL_HANDLE_WIDTH(), slider_size),
      scroll_bar->width());
  }
  auto slider_size = (scroll_bar->pageStep() * scroll_bar->height()) /
    (scroll_bar->maximum() - scroll_bar->minimum() + scroll_bar->pageStep());
  return std::min(std::max(MINIMUM_VERTICAL_HANDLE_HEIGHT(), slider_size),
    scroll_bar->height());
}

int ScrollBarStyle::get_horizontal_slider_position(
    const QScrollBar* scroll_bar) const {
  return sliderPositionFromValue(scroll_bar->minimum(),
    scroll_bar->maximum(), scroll_bar->value(),
    scroll_bar->width() - get_handle_size(scroll_bar));
}

int ScrollBarStyle::get_vertical_slider_position(
    const QScrollBar* scroll_bar) const {
  return sliderPositionFromValue(scroll_bar->minimum(),
    scroll_bar->maximum(), scroll_bar->value(),
    scroll_bar->height() - get_handle_size(scroll_bar));
}
