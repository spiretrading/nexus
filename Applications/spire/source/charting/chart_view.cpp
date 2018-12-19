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
  QVariant to_variant(ChartValue::Type type, ChartValue value) {
    if(type == ChartValue::Type::DURATION) {
      return QVariant::fromValue(static_cast<time_duration>(value));
    } else if(type == ChartValue::Type::MONEY) {
      return QVariant::fromValue(static_cast<Money>(value));
    } else if(type == ChartValue::Type::QUANTITY) {
      return QVariant::fromValue(static_cast<Quantity>(value));
    } else if(type == ChartValue::Type::TIMESTAMP) {
      return QVariant::fromValue(static_cast<ptime>(value));
    }
    return QVariant();
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
  painter.setFont(m_label_font);
  painter.setPen(Qt::white);
  auto font_metrics = QFontMetrics(m_label_font);
  auto x_origin = width() -
    (font_metrics.width("M") *
      QString::number(
        static_cast<double>(
          static_cast<Quantity>(ChartValue(m_top_left.m_y))),
        'f', 2).length()) + scale_width(8);
  auto y_origin = height() - (font_metrics.height() + scale_height(9));
  painter.drawLine(x_origin, 0, x_origin, y_origin);
  painter.drawLine(0, y_origin, x_origin, y_origin);
  auto x_range = m_bottom_right.m_x - m_top_left.m_x;
  auto x_step = get_step(m_x_axis_type, x_range);
  auto y_range = m_top_left.m_y - m_bottom_right.m_y;
  auto y_step = get_step(m_y_axis_type, y_range);
  if(x_range <= x_step || y_range <= y_step) {
    return;
  }
  auto y_step_count = y_range / y_step;
  auto y_pixel_step = y_origin / y_step_count;
  if((y_step_count - 1) * y_pixel_step < height()) {
    y_pixel_step += 1;
  }
  auto y_value = m_bottom_right.m_y;
  auto y = y_origin;
  while(y_value <= m_top_left.m_y) {
    y_value += y_step;
    y -= y_pixel_step;
    painter.setPen("#3A3348");
    painter.drawLine(0, y, x_origin, y);
    painter.setPen(Qt::white);
    painter.drawLine(x_origin, y, x_origin + scale_width(2), y);
    painter.drawText(x_origin + scale_width(3),
      y + (font_metrics.height() / 3),
        m_item_delegate->displayText(to_variant(m_y_axis_type, y_value),
          QLocale()));
  }
  auto x_step_count = x_range / x_step;
  auto x_pixel_step = x_origin / (x_range / x_step);
  if((x_step_count - 1) * x_pixel_step < width()) {
    x_pixel_step += 1;
  }
  auto x_text_width = font_metrics.width(drawable_timestamp(
    static_cast<ptime>(m_top_left.m_x)));
  auto x_value = m_bottom_right.m_x;
  auto x = x_origin;
  while(x_value >= m_top_left.m_x) {
    x_value -= x_step;
    x -= x_pixel_step;
    painter.setPen("#3A3348");
    painter.drawLine(x, 0, x, y_origin);
    painter.setPen(Qt::white);
    painter.drawLine(x, y_origin, x, y_origin + scale_height(2));
    painter.drawText(x - x_text_width / 2,
      y_origin + font_metrics.height() + scale_height(2),
      drawable_timestamp(static_cast<ptime>(x_value)));
  }
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
