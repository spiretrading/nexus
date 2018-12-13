#include "spire/charting/chart_view.hpp"
#include <iostream>
#include <locale>
#include <boost/date_time/posix_time/posix_time.hpp>
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
  auto money_step = value_to_pixel(range_y.front(), range_y.back(),
    range_y.front(), height() - (height() - origin_y)) + value_to_pixel(
      range_y.front(), range_y.back(), range_y[1],
      height() - (height() - origin_y));
  for(auto i = 0; i < range_y.size(); ++i) {
    auto y = origin_y - (money_step * i) - money_step;
    painter.drawLine(0, y, origin_x, y);
    painter.drawLine(origin_x, y, origin_x + scale_width(2), y);
    painter.drawText(origin_x + scale_width(3),
      y + (y_axis_fm.height() / 3),
      QString::number(static_cast<double>(static_cast<Quantity>(range_y[i])),
        'f', 2));
  }
  auto time_step = value_to_pixel(range_x.front(), range_x.back(),
    range_x.front(), width() - (width() - origin_x)) +
    value_to_pixel(range_x.front(), range_x.back(), range_x[1],
      width() - (width() - origin_x));
  if(range_x.size() * time_step < width()) {
    time_step += 1;
  }
  auto x_axis_fm = QFontMetrics(m_label_font);
  auto timestamp_width = (y_axis_fm.width("M") * QString::number(
    static_cast<double>(static_cast<Quantity>(range_y.front())), 'f', 2)
    .length());
  for(auto i = 0; i < range_x.size(); ++i) {
    auto x = origin_x - (time_step * i) - time_step;
    painter.drawLine(x, 0, x, origin_y);
    painter.drawLine(x, origin_y, x, origin_y + scale_height(2));
    painter.drawText(x - timestamp_width / 2.5, origin_y + x_axis_fm.height() +
      scale_height(2),
      drawable_timestamp(static_cast<ptime>(range_x[i])));
  }
}

std::vector<ChartValue> ChartView::get_axis_values(
    const ChartValue::Type& type, const ChartValue& range_start,
    const ChartValue& range_end) {
  auto values = std::vector<ChartValue>();
  auto range = range_end - range_start;
  auto step = get_step(type, range);
  auto value = range_start + step;
  while(value < range_end - step) {
    values.push_back(ChartValue(value));
    value += step;
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

QString ChartView::drawable_timestamp(const ptime& time) {
  std::ostringstream ss;
  ss.imbue(std::locale(std::cout.getloc(),
    new boost::posix_time::time_facet("%H:%M:%S")));
  ss << time;
  return QString::fromStdString(ss.str());
}

ChartValue ChartView::get_step(const ChartValue::Type& value_type,
    const ChartValue& range) {
  auto step = ChartValue();
  if(value_type == ChartValue::Type::MONEY) {
    auto money_range = static_cast<Money>(range);
    if(money_range <= Money(0.01)) {
      return ChartValue(Money(0.001));
    } else if(money_range <= Money(0.1)) {
      return ChartValue(Money(0.01));
    } else if(money_range <= Money(1)) {
      return ChartValue(Money(0.1));
    } else {
      return ChartValue(Money(1));
    }
  } else if(value_type == ChartValue::Type::TIMESTAMP) {
    auto time_range = static_cast<time_duration>(range);
    if(time_range <= time_duration(0, 1, 0)) {
      return ChartValue(time_duration(0, 0, 1));
    } else if(time_range <= time_duration(1, 0, 0)) {
      return ChartValue(time_duration(0, 1, 0));
    } else if(time_range <= time_duration(24, 0, 0)) {
      return ChartValue(time_duration(1, 0, 0));
    } else {
      return ChartValue(time_duration(24, 0, 0));
    }
  }
  return ChartValue();
}
