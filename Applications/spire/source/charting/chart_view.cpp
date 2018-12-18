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
      m_y_axis_type(y_axis_type),
      m_timestamp_format("%H:%M:%S") {
  setFocusPolicy(Qt::NoFocus);
  m_label_font = QFont("Roboto");
  m_label_font.setPixelSize(scale_height(10));
  m_x_axis_type = ChartValue::Type::TIMESTAMP;
  m_y_axis_type = ChartValue::Type::MONEY;
  auto current_time = boost::posix_time::second_clock::local_time();
  auto bottom_right = ChartPoint(
    ChartValue(current_time),
    ChartValue(Nexus::Money(10)));
  auto top_left = ChartPoint(
    ChartValue(current_time - boost::posix_time::hours(1)),
    ChartValue(Nexus::Money(10.10)));
  set_region(top_left, bottom_right);
}

void ChartView::set_region(ChartPoint top_left, ChartPoint bottom_right) {
  m_top_left = top_left;
  m_bottom_right = bottom_right;
}

void ChartView::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.setPen(Qt::white);
  painter.setFont(m_label_font);
  auto y_values = get_axis_values(m_y_axis_type,
    m_bottom_right.m_y, m_top_left.m_y);
  auto font_metrics = QFontMetrics(m_label_font);
  auto origin_x = width() -
    (font_metrics.width("M") *
      QString::number(
        static_cast<double>(
          static_cast<Quantity>(y_values.front())), 'f', 2).length()) +
      scale_width(8);
  auto origin_y = height() - scale_height(20);
  painter.drawLine(origin_x, 0, origin_x, origin_y);
  painter.drawLine(0, origin_y, origin_x, origin_y);
  auto money_step = (height() - (height() - origin_y)) / (y_values.size() - 1);
  if((y_values.size() - 1) * money_step < height()) {
    money_step += 1;
  }
  for(auto i = 1; i < y_values.size() - 1; ++i) {
    auto y = origin_y - (money_step * (i - 1)) - money_step;
    painter.drawLine(0, y, origin_x + scale_width(2), y);
    painter.drawText(origin_x + scale_width(3),
      y + (font_metrics.height() / 3),
      QString::number(static_cast<double>(static_cast<Quantity>(y_values[i])),
        'f', 2));
  }
  auto x_values = get_axis_values(m_x_axis_type, m_top_left.m_x,
    m_bottom_right.m_x);
  std::reverse(x_values.begin(), x_values.end());
  auto time_step = (width() - (width() - origin_x)) / (x_values.size() - 1);
  if((x_values.size() - 1) * time_step < width()) {
    time_step += 1;
  }
  auto timestamp_width = (font_metrics.width("M") * QString::number(
    static_cast<double>(static_cast<Quantity>(y_values.front())), 'f', 2)
    .length());
  for(auto i = 0; i < x_values.size() - 1; ++i) {
    auto x = origin_x - (time_step * i) - time_step;
    painter.drawLine(x, 0, x, origin_y + scale_height(2));
    painter.drawText(x - timestamp_width / 2.5,
      origin_y + font_metrics.height() + scale_height(2),
      drawable_timestamp(static_cast<ptime>(x_values[i])));
  }
}

std::vector<ChartValue> ChartView::get_axis_values(
    const ChartValue::Type& type, const ChartValue& range_start,
    const ChartValue& range_end) {
  auto values = std::vector<ChartValue>();
  auto range = range_end - range_start;
  auto step = get_step(type, range);
  auto value = range_start;
  while(value <= range_end) {
    values.push_back(ChartValue(value));
    value += step;
  }
  return values;
}

QString ChartView::drawable_timestamp(const ptime& time) {
  std::ostringstream ss;
  ss.imbue(std::locale(std::cout.getloc(),
    new boost::posix_time::time_facet(m_timestamp_format.c_str())));
  ss << time;
  return QString::fromStdString(ss.str());
}

ChartValue ChartView::get_step(const ChartValue::Type& value_type,
    const ChartValue& range) {
  auto step = ChartValue();
  if(value_type == ChartValue::Type::MONEY) {
    auto money_range = static_cast<Money>(range);
    if(money_range <= Money::CENT) {
      return ChartValue(Money::CENT / 10);
    } else if(money_range <= 10 * Money::CENT) {
      return ChartValue(Money::CENT);
    } else if(money_range <= Money::ONE) {
      return ChartValue(10 * Money::CENT);
    } else {
      return ChartValue(Money::ONE);
    }
  } else if(value_type == ChartValue::Type::TIMESTAMP) {
    auto time_range = static_cast<time_duration>(range);
    if(time_range <= time_duration(0, 1, 0)) {
      m_timestamp_format = "%H:%M:%S";
      return ChartValue(time_duration(0, 0, 1));
    } else if(time_range <= time_duration(1, 0, 0)) {
      m_timestamp_format = "%H:%M";
      return ChartValue(time_duration(0, 1, 0));
    } else if(time_range <= time_duration(24, 0, 0)) {
      m_timestamp_format = "%H";
      return ChartValue(time_duration(1, 0, 0));
    } else {
      m_timestamp_format = "%H";
      return ChartValue(time_duration(24, 0, 0));
    }
  }
  return ChartValue();
}
