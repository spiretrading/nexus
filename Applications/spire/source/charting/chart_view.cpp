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

ChartView::ChartView(ChartValue::Type x_axis_type, ChartValue::Type y_axis_type,
    QWidget* parent)
    : QWidget(parent),
      m_x_axis_type(x_axis_type),
      m_y_axis_type(y_axis_type),
      m_timestamp_format("%H:%M:%S"),
      m_label_font("Roboto"),
      m_item_delegate(new CustomVariantItemDelegate(this)) {
  setFocusPolicy(Qt::NoFocus);
  setMouseTracking(true);
  setAttribute(Qt::WA_Hover);
  m_label_font.setPixelSize(scale_height(10));
  auto current_time = boost::posix_time::second_clock::local_time();
  auto bottom_right = ChartPoint(ChartValue(current_time),
    ChartValue(Nexus::Money(10)));
  auto top_left = ChartPoint(
    ChartValue(current_time - boost::posix_time::hours(1)),
    ChartValue(Nexus::Money(10.10)));
  set_region(top_left, bottom_right);
}

ChartPoint ChartView::convert_pixels_to_chart(const QPoint& point) const {
  return {};
}

QPoint ChartView::convert_chart_to_pixels(const ChartPoint& point) const {
  return {};
}

void ChartView::set_crosshair(const ChartPoint& position) {
}

void ChartView::set_crosshair(const QPoint& position) {
  m_crosshair_pos = position;
}

void ChartView::reset_crosshair() {
  m_crosshair_pos = QPoint();
}

void ChartView::set_region(const ChartPoint& top_left,
    const ChartPoint& bottom_right) {
  m_top_left = top_left;
  m_bottom_right = bottom_right;
  update();
}

void ChartView::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.setFont(m_label_font);
  painter.setPen(Qt::white);
  auto font_metrics = QFontMetrics(m_label_font);
  auto x_origin = width() - (font_metrics.width("M") * (get_string(
    m_y_axis_type, m_top_left.m_y).length() + 1)) + scale_width(8);
  auto y_origin = height() - (font_metrics.height() + scale_height(9));
  painter.drawLine(x_origin, 0, x_origin, y_origin);
  painter.drawLine(0, y_origin, x_origin, y_origin);
  auto x_range = m_bottom_right.m_x - m_top_left.m_x;
  auto x_step = calculate_step(m_x_axis_type, x_range);
  auto y_range = m_top_left.m_y - m_bottom_right.m_y;
  auto y_step = calculate_step(m_y_axis_type, y_range);
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
    painter.drawText(x_origin + scale_width(3), y + (font_metrics.height() / 3),
      m_item_delegate->displayText(to_variant(m_y_axis_type, y_value),
      QLocale()));
  }
  auto x_step_count = x_range / x_step;
  auto x_pixel_step = x_origin / (x_range / x_step);
  if((x_step_count - 1) * x_pixel_step < width()) {
    x_pixel_step += 1;
  }
  auto x_text_width = font_metrics.width(get_string(m_x_axis_type,
    m_top_left.m_x));
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
      get_string(m_x_axis_type, x_value));
  }
  if(!m_crosshair_pos.isNull()) {
    painter.fillRect(event->rect(), Qt::red);
  }
}

ChartValue ChartView::calculate_step(ChartValue::Type value_type,
    ChartValue range) {
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
    if(time_range <= minutes(1)) {
      m_timestamp_format = "%H:%M:%S";
      return ChartValue(seconds(1));
    } else if(time_range <= hours(1)) {
      m_timestamp_format = "%H:%M";
      return ChartValue(minutes(1));
    } else if(time_range <= hours(24)) {
      m_timestamp_format = "%H";
      return ChartValue(hours(1));
    } else {
      m_timestamp_format = "%H";
      return ChartValue(hours(24));
    }
  }
  return ChartValue();
}

QString ChartView::get_string(ChartValue::Type type, ChartValue value) const {
  if(type == ChartValue::Type::DURATION) {
    std::ostringstream ss;
    ss << static_cast<time_duration>(value);
    return QString::fromStdString(ss.str());
  } else if(type == ChartValue::Type::MONEY) {
    return QString::number(static_cast<double>(static_cast<Money>(value)));
  } else if(type == ChartValue::Type::QUANTITY) {
    return QString::number(static_cast<double>(static_cast<Quantity>(value)));
  } else if(type == ChartValue::Type::TIMESTAMP) {

    // TODO: convert from ptime to QTime
    std::ostringstream ss;
    ss.imbue(std::locale(std::cout.getloc(),
      new boost::posix_time::time_facet(m_timestamp_format.c_str())));
    ss << static_cast<ptime>(value);
    return QString::fromStdString(ss.str());
  }
  return QString();
}
