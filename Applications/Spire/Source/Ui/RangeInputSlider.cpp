#include "Spire/Ui/RangeInputSlider.hpp"
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "spire/Spire/Utility.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  int HANDLE_SIZE() {
    return scale_width(16);
  }

  int MARGIN() {
    return scale_width(8);
  }

  int RANGE_MARGIN() {
    return MARGIN() + HANDLE_SIZE();
  }
}

RangeInputSlider::RangeInputSlider(Scalar min_value, Scalar max_value,
    Scalar step, QWidget* parent)
    : QWidget(parent),
      m_min_value(min_value),
      m_max_value(max_value),
      m_step(step),
      m_is_dragging_handle(false),
      m_is_dragging_min(false),
      m_is_dragging_max(false),
      m_is_hovering_handle(false),
      m_is_hovering_min(false),
      m_is_hovering_max(false),
      m_mouse_offset(0) {
  setMouseTracking(true);
  setFixedHeight(scale_height(26));
  m_current_min_value = rounded_value(min_value);
  m_current_max_value = rounded_value(max_value);
  move_min_handle(m_current_min_value);
  move_max_handle(m_current_max_value);
}

void RangeInputSlider::set_min_value(Scalar value) {
  m_current_min_value = rounded_value(value);
  move_min_handle(m_current_min_value);
}

void RangeInputSlider::set_max_value(Scalar value) {
  m_current_max_value = rounded_value(value);
  move_max_handle(m_current_max_value);
}

void RangeInputSlider::set_histogram(
    const RangeInputModel::Histogram& histogram) {
  m_histogram = histogram;
}

connection RangeInputSlider::connect_min_changed_signal(
    const ChangedSignal::slot_type& slot) const {
  return m_min_changed_signal.connect(slot);
}

connection RangeInputSlider::connect_max_changed_signal(
    const ChangedSignal::slot_type& slot) const {
  return m_max_changed_signal.connect(slot);
}

void RangeInputSlider::leaveEvent(QEvent* event) {
  m_is_hovering_handle = false;
  m_is_hovering_min = false;
  m_is_hovering_max = false;
  update();
}

void RangeInputSlider::mouseMoveEvent(QMouseEvent* event) {
  auto x = event->x();
  if(m_is_dragging_handle) {
    auto delta = x - m_last_mouse_x;
    if(m_min_handle_x + delta < RANGE_MARGIN()) {
      m_max_handle_x = RANGE_MARGIN() + m_max_handle_x - m_min_handle_x;
      m_min_handle_x = MARGIN() + HANDLE_SIZE();
    } else if(m_max_handle_x + delta > width() - RANGE_MARGIN()) {
      m_min_handle_x = width() - MARGIN() - HANDLE_SIZE() -
        (m_max_handle_x - m_min_handle_x);
      m_max_handle_x = width() - MARGIN() - HANDLE_SIZE();
    } else {
      m_min_handle_x += delta;
      m_max_handle_x += delta;
    }
    m_last_mouse_x = x;
    m_current_min_value = rounded_value(map_x_to_value(m_min_handle_x));
    m_min_changed_signal(m_current_min_value);
    m_current_max_value = rounded_value(map_x_to_value(m_max_handle_x));
    m_max_changed_signal(m_current_max_value);
  } else if(m_is_dragging_min) {
    auto pos_x = max(RANGE_MARGIN(), x - m_mouse_offset);
    pos_x = min(pos_x, m_max_handle_x);
    m_min_handle_x = pos_x;
    m_current_min_value = rounded_value(map_x_to_value(m_min_handle_x));
    m_min_changed_signal(m_current_min_value);
  } else if(m_is_dragging_max) {
    auto pos_x = min(x - m_mouse_offset,
      width() - MARGIN() - HANDLE_SIZE());
    pos_x = max(m_min_handle_x, pos_x);
    m_max_handle_x = pos_x;
    m_current_max_value = rounded_value(map_x_to_value(m_max_handle_x));
    m_max_changed_signal(m_current_max_value);
  }
  if(x > m_min_handle_x  - HANDLE_SIZE() && x < m_min_handle_x) {
    m_is_hovering_min = true;
  } else {
    m_is_hovering_min = false;
  }
  if(x > m_max_handle_x && x < m_max_handle_x + HANDLE_SIZE()) {
    m_is_hovering_max = true;
  } else {
    m_is_hovering_max = false;
  }
  if(x > m_min_handle_x && x < m_max_handle_x) {
    m_is_hovering_handle = true;
  } else {
    m_is_hovering_handle = false;
  }
  update();
}

void RangeInputSlider::mousePressEvent(QMouseEvent* event) {
  auto x = event->x();
  if(x > m_min_handle_x && x < m_max_handle_x) {
    m_last_mouse_x = x;
    m_is_dragging_handle = true;
  } else if(x > m_min_handle_x - HANDLE_SIZE() && x < m_min_handle_x) {
    m_mouse_offset = x - m_min_handle_x;
    m_is_dragging_min = true;
  } else if(x > m_max_handle_x && x < m_max_handle_x + HANDLE_SIZE()) {
    m_mouse_offset = x - m_max_handle_x;
    m_is_dragging_max = true;
  }
}

void RangeInputSlider::mouseReleaseEvent(QMouseEvent* event) {
  m_is_dragging_handle = false;
  m_is_dragging_min = false;
  m_is_dragging_max = false;
  m_mouse_offset = 0;
  update();
}

void RangeInputSlider::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  for(auto i = size_t(0); i < m_histogram.m_histogram.size(); ++i) {
    if(m_histogram.m_histogram[i] > 0) {
      auto bin_height = scale_height(26) * m_histogram.m_histogram[i] /
        m_histogram.m_highest_frequency;
      painter.fillRect(i * 2 + scale_width(24),
        scale_height(26) - bin_height, scale_width(1), bin_height,
        QColor("#C8C8C8"));
    }
  }
  painter.setPen(QColor("#684BC7"));
  painter.setBrush(Qt::white);
  painter.drawRect(HANDLE_SIZE(), scale_width(11),
    m_min_handle_x, scale_height(4));
  painter.fillRect(m_min_handle_x, scale_width(11),
    m_max_handle_x - m_min_handle_x, scale_height(5), QColor("#684BC7"));
  painter.drawRect(m_max_handle_x, scale_width(11),
    width() - m_max_handle_x - HANDLE_SIZE(), scale_height(4));
  draw_handle(painter, m_is_hovering_min || m_is_dragging_min ||
    m_is_dragging_handle || m_is_hovering_handle, m_min_handle_x -
    HANDLE_SIZE());
  draw_handle(painter, m_is_hovering_max || m_is_dragging_max ||
    m_is_dragging_handle || m_is_hovering_handle, m_max_handle_x);
}

void RangeInputSlider::resizeEvent(QResizeEvent* event) {
  move_min_handle(m_current_min_value);
  move_max_handle(m_current_max_value);
}

void RangeInputSlider::draw_handle(QPainter& painter, bool is_highlighted,
    int x) {
  painter.setPen(Qt::white);
  painter.setBrush(Qt::white);
  painter.drawEllipse(x, scale_height(5), HANDLE_SIZE(),
    HANDLE_SIZE());
  if(is_highlighted) {
    painter.setPen(QColor("#4B23A0"));
    painter.setBrush(QColor("#4B23A0"));
    painter.drawEllipse(x + scale_width(2), scale_height(7),
      scale_width(12) - 1, scale_height(12) - 1);
  } else {
    painter.setPen(QColor("#684BC7"));
    painter.setBrush(QColor("#684BC7"));
    painter.drawEllipse(x + scale_width(2), scale_height(7),
      scale_width(12) - 1, scale_height(12) - 1);
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    painter.drawEllipse(x + scale_width(4), scale_height(9),
      scale_width(8) - 1, scale_height(8) - 1);
  }
}

Scalar RangeInputSlider::map_x_to_value(int x) {
  return map_to(static_cast<double>(x),
    static_cast<double>(RANGE_MARGIN()),
    static_cast<double>(width() - MARGIN() - HANDLE_SIZE()),
    m_min_value, m_max_value);
}

void RangeInputSlider::move_min_handle(Scalar value) {
  if(value == m_current_max_value) {
    m_min_handle_x = m_max_handle_x;
  } else {
    m_min_handle_x = static_cast<int>(map_to(value, m_min_value, m_max_value,
      static_cast<double>(RANGE_MARGIN()),
      static_cast<double>(width() - MARGIN() - HANDLE_SIZE())));
  }
  update();
}

void RangeInputSlider::move_max_handle(Scalar value) {
  if(value == m_current_min_value) {
    m_max_handle_x = m_min_handle_x;
  } else {
    m_max_handle_x = static_cast<int>(map_to(value, m_min_value, m_max_value,
      static_cast<double>(RANGE_MARGIN()),
      static_cast<double>(width() - MARGIN() - HANDLE_SIZE())));
  }
  update();
}

Scalar RangeInputSlider::rounded_value(Scalar value) {
  auto result = value + (m_step / 2);
  result -= result % m_step;
  return result;
}
