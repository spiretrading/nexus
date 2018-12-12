#include "spire/charting/chart_view.hpp"
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include "Nexus/Definitions/Money.hpp"
#include "spire/spire/dimensions.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

ChartView::ChartView(ChartValue::Type x_axis_type,
    ChartValue::Type y_axis_type, QWidget* parent)
    : QWidget(parent),
      m_x_axis_type(x_axis_type),
      m_y_axis_type(y_axis_type) {
  m_label_font = QFont("Roboto");
  m_label_font.setPixelSize(scale_height(10));
}

void ChartView::set_region(ChartPoint top_left, ChartPoint bottom_right) {
  m_top_left = top_left;
  m_bottom_right = bottom_right;
}

void ChartView::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.fillRect(event->rect(), "#25212E");
  painter.setPen(Qt::white);
  painter.setFont(m_label_font);
  auto range_x = get_axis_values(m_x_axis_type,
    m_top_left.m_x, m_bottom_right.m_x);
  auto range_y = get_axis_values(m_y_axis_type,
    m_bottom_right.m_y, m_top_left.m_y);
  auto y_axis_fm = QFontMetrics(m_label_font);
  auto origin_x = width() -
    (y_axis_fm.width("M") *
      QString::number(
        static_cast<double>(
          static_cast<Quantity>(range_y.front())), 'f', 2).length()) +
      scale_width(8);
  auto origin_y = height() - scale_height(20);
  painter.drawLine(origin_x, 0, origin_x, origin_y);
  painter.drawLine(0, origin_y, origin_x, origin_y);
  auto step =  value_to_pixel(range_y.front(), range_y.back(), range_y.front(),
      height()) + value_to_pixel(range_y.front(), range_y.back(), range_y[1],
        height());
  for(auto i = 0; i < range_y.size(); ++i) {
    auto y = origin_y - (step * i) - step;
    painter.drawLine(0, y, origin_x, y);
    painter.drawLine(origin_x, y, origin_x + scale_width(2), y);
    painter.drawText(origin_x + scale_width(3),
      y + (y_axis_fm.height() / 3),
      QString::number(static_cast<double>(static_cast<Quantity>(range_y[i])),
        'f', 2));
  }
  for(auto& value : range_x) {
    auto pixel = value_to_pixel(range_x.front(), range_x.back(), value,
      width());
    painter.drawLine(pixel, 0, pixel, origin_y);
  }
}

std::vector<ChartValue> ChartView::get_axis_values(
    const ChartValue::Type& type, const ChartValue& range_start,
    const ChartValue& range_end) {
  auto values = std::vector<ChartValue>();
  if(type == ChartValue::Type::MONEY) {
    auto money_start = static_cast<Money>(range_start);
    auto money_end = static_cast<Money>(range_end);
    auto range = money_end - money_start;
    auto step = Money();
    if(range <= Money(0.01)) {
      step = Money(0.001);
    } else if(range <= Money(0.1)) {
      step = Money(0.01);
    } else if(range <= Money(1)) {
      step = Money(0.1);
    } else {
      step = Money(1);
    }
    auto value = money_start + step;
    while(value < money_end - step) {
      values.push_back(ChartValue(value));
      value += step;
    }
  } else if(type == ChartValue::Type::TIMESTAMP) {
    auto time_start = static_cast<ptime>(range_start);
    auto time_end = static_cast<ptime>(range_end);
  }
  return values;
}

int ChartView::value_to_pixel(const ChartValue& range_start,
    const ChartValue& range_end, const ChartValue& value,
    int widget_size) {
  auto start_quantity = static_cast<Quantity>(range_start);
  auto end_quantity = static_cast<Quantity>(range_end);
  auto value_quantity = static_cast<Quantity>(value);
  return static_cast<int>(((value_quantity - start_quantity) *
    (widget_size / (end_quantity - start_quantity))));
}
