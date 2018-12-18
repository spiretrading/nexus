#include "spire/charting/chart_view.hpp"
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

namespace {
  QVariant chart_value_to_variant(ChartValue::Type type,
      const ChartValue& value) {
    return QVariant::fromValue(static_cast<Money>(value));
  }
}

ChartView::ChartView(ChartValue::Type x_axis_type,
    ChartValue::Type y_axis_type, QWidget* parent)
    : QWidget(parent),
      m_x_axis_type(x_axis_type),
      m_y_axis_type(y_axis_type),
      m_timestamp_format("%H:%M:%S") {
  setFocusPolicy(Qt::NoFocus);
  m_label_font = QFont("Roboto");
  m_label_font.setPixelSize(scale_height(10));
  m_item_delegate = new CustomVariantItemDelegate(this);
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
  update();
}

void ChartView::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.setPen(Qt::white);
  painter.setFont(m_label_font);
  auto y_values = get_axis_values(m_y_axis_type,
    m_bottom_right.m_y, m_top_left.m_y);
  auto x_values = get_axis_values(m_x_axis_type, m_top_left.m_x,
    m_bottom_right.m_x);
  if(y_values.size() == 0 || x_values.size() == 0) {
    return;
  }
  auto font_metrics = QFontMetrics(m_label_font);
  auto x_origin = width() -
    (font_metrics.width("M") *
      QString::number(
        static_cast<double>(
          static_cast<Quantity>(y_values.front())), 'f', 2).length()) +
      scale_width(8);
  auto y_origin = height() - scale_height(20);
  painter.drawLine(x_origin, 0, x_origin, y_origin);
  painter.drawLine(0, y_origin, x_origin, y_origin);
  auto y_step = y_origin / (y_values.size() - 1);
  if((y_values.size() - 1) * y_step < height()) {
    y_step += 1;
  }
  for(auto i = 1; i < y_values.size() - 1; ++i) {
    auto y = y_origin - i * y_step;
    painter.setPen("#3A3348");
    painter.drawLine(0, y, x_origin, y);
    painter.setPen(Qt::white);
    painter.drawLine(x_origin, y, x_origin + scale_width(2), y);
    painter.drawText(x_origin + scale_width(3),
      y + (font_metrics.height() / 3), 
        m_item_delegate->displayText(chart_value_to_variant(
          ChartValue::Type::MONEY, y_values[i]), QLocale()));
  }
  std::reverse(x_values.begin(), x_values.end());
  auto x_step = x_origin / (x_values.size() - 1);
  if((x_values.size() - 1) * x_step < width()) {
    x_step += 1;
  }
  auto x_text_width = font_metrics.width(drawable_timestamp(
    static_cast<ptime>(x_values.front())));
  for(auto i = 1; i < x_values.size() - 1; ++i) {
    auto x = x_origin - i * x_step;
    painter.setPen("#3A3348");
    painter.drawLine(x, 0, x, y_origin);
    painter.setPen(Qt::white);
    painter.drawLine(x, y_origin, x, y_origin + scale_height(2));
    painter.drawText(x - x_text_width / 2,
      y_origin + font_metrics.height() + scale_height(2),
      drawable_timestamp(static_cast<ptime>(x_values[i])));
  }
}

std::vector<ChartValue> ChartView::get_axis_values(
    ChartValue::Type type, ChartValue range_start, ChartValue range_end) {
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
