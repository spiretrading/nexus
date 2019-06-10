#include "Spire/Charting/ChartView.hpp"
#include <locale>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include "Nexus/Definitions/Money.hpp"
#include "Spire/Charting/ChartModel.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto LINE_HOVER_DISTANCE = 25;

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

  template<typename T, typename U>
  U map_to(T value, T a, T b, U c, U d) {
    return static_cast<U>(((value - a) / (b - a)) * (d - c) + c);
  }

  ChartValue calculate_step(ChartValue::Type value_type,
      ChartValue range) {
    if(value_type == ChartValue::Type::MONEY) {
      // TODO: compiler bug workaround
      return ChartValue(Money::FromValue("0.1").get());
    } else if(value_type == ChartValue::Type::TIMESTAMP) {
      return ChartValue(minutes(10));
    }
    return ChartValue();
  }
}

ChartView::ChartView(ChartModel& model, QWidget* parent)
    : QWidget(parent),
      m_model(&model),
      m_x_origin(0),
      m_label_font("Roboto"),
      m_font_metrics(QFont()),
      m_item_delegate(new CustomVariantItemDelegate(this)),
      m_dashed_line_pen(QColor("#E5E5E5"), scale_width(1), Qt::CustomDashLine),
      m_label_text_color(QColor("#25212E")),
      m_is_auto_scaled(true),
      m_draw_state(DrawState::OFF),
      m_mouse_buttons(Qt::NoButton),
      m_multi_select(false) {
  setFocusPolicy(Qt::NoFocus);
  setMouseTracking(true);
  setAttribute(Qt::WA_Hover);
  m_label_font.setPixelSize(scale_height(10));
  m_font_metrics = QFontMetrics(m_label_font);
  m_dashed_line_pen.setDashPattern({static_cast<double>(scale_width(3)),
    static_cast<double>(scale_width(3))});
  m_crosshair_cursor = QCursor(QPixmap::fromImage(
    imageFromSvg(":/Icons/chart-cursor.svg", scale(18, 18))));
  m_hand_cursor = QCursor(QPixmap::fromImage(
    imageFromSvg(":/Icons/finger-cursor.svg", scale(18, 18))), 0, 0);
}

ChartPoint ChartView::convert_pixels_to_chart(const QPoint& point) const {
  return ChartPoint(
    map_to(static_cast<double>(point.x()), 0.0,
      static_cast<double>(m_x_origin), m_top_left.m_x, m_bottom_right.m_x),
    map_to(static_cast<double>(point.y()), static_cast<double>(m_y_origin),
      0.0, m_bottom_right.m_y, m_top_left.m_y));
}

QPoint ChartView::convert_chart_to_pixels(const ChartPoint& point) const {
  return QPoint(
    static_cast<int>(map_to(point.m_x, m_top_left.m_x, m_bottom_right.m_x, 0.0,
      static_cast<double>(m_x_origin))),
    static_cast<int>(map_to(point.m_y, m_bottom_right.m_y, m_top_left.m_y,
      static_cast<double>(m_y_origin), 0.0)));
}

void ChartView::set_crosshair(const ChartPoint& position,
    Qt::MouseButtons buttons) {
  set_crosshair(convert_chart_to_pixels(position), buttons);
}

void ChartView::set_crosshair(const QPoint& position,
    Qt::MouseButtons buttons) {
  if(m_draw_state != DrawState::OFF) {
    if(buttons.testFlag(Qt::LeftButton) !=
        m_mouse_buttons.testFlag(Qt::LeftButton)) {
      if(buttons.testFlag(Qt::LeftButton)) {
        on_left_mouse_button_press(position);
      } else {
        on_left_mouse_button_release();
      }
    }
    if(buttons.testFlag(Qt::RightButton) !=
        m_mouse_buttons.testFlag(Qt::RightButton)) {
      if(buttons.testFlag(Qt::RightButton)) {
        on_right_mouse_button_press();
      }
    }
  }
  m_mouse_buttons = buttons;
  if(m_crosshair_pos) {
    m_last_crosshair_pos = *m_crosshair_pos;
  } else {
    m_last_crosshair_pos = position;
  }
  m_crosshair_pos = position;
  if(m_draw_state != DrawState::OFF) {
    if(m_draw_state == DrawState::LINE || m_draw_state == DrawState::NEW ||
        m_draw_state == DrawState::POINT) {
      if(!QRect(0, 0, m_x_origin, m_y_origin).contains(*m_crosshair_pos)) {
        m_draw_state = DrawState::IDLE;
      }
    }
    if(m_draw_state == DrawState::IDLE) {
      m_current_trend_line_id = update_intersection(*m_crosshair_pos);
      if(m_current_trend_line_id != -1) {
        m_draw_state = DrawState::HOVER;
      }
    } else if(m_draw_state == DrawState::HOVER) {
      m_current_trend_line_id = update_intersection(*m_crosshair_pos);
      if(m_current_trend_line_id == -1) {
        m_draw_state = DrawState::IDLE;
      }
    } else if(m_draw_state == DrawState::LINE) {
      auto line = m_trend_line_model.get(m_current_trend_line_id);
      auto first = std::get<0>(line.m_points);
      auto second = std::get<1>(line.m_points);
      auto delta = chart_delta(m_last_crosshair_pos, *m_crosshair_pos);
      line.m_points = {{first.m_x - delta.m_x, first.m_y - delta.m_y},
        {second.m_x - delta.m_x, second.m_y - delta.m_y}};
      m_trend_line_model.update(line, m_current_trend_line_id);
    } else if(m_draw_state == DrawState::NEW) {
      auto line = m_trend_line_model.get(m_current_trend_line_id);
      m_current_trend_line_point = convert_pixels_to_chart(*m_crosshair_pos);
      m_trend_line_model.update(TrendLine{{m_current_trend_line_point,
        m_current_stationary_point}, line.m_color, line.m_style},
        m_current_trend_line_id);
    } else if(m_draw_state == DrawState::POINT) {
      auto line = m_trend_line_model.get(m_current_trend_line_id);
      m_current_trend_line_point = convert_pixels_to_chart(*m_crosshair_pos);
      m_trend_line_model.update(TrendLine{{m_current_trend_line_point,
        m_current_stationary_point}, line.m_color, line.m_style},
        m_current_trend_line_id);
    }
  }
  update();
}

void ChartView::reset_crosshair() {
  m_crosshair_pos.reset();
}

std::tuple<ChartPoint, ChartPoint> ChartView::get_region() const {
  return {m_top_left, m_bottom_right};
}

void ChartView::set_region(const ChartPoint& top_left,
    const ChartPoint& bottom_right) {
  if(std::tie(m_top_left.m_x, m_top_left.m_y) ==
      std::tie(top_left.m_x, top_left.m_y) &&
      std::tie(m_bottom_right.m_x, m_bottom_right.m_y) ==
      std::tie(bottom_right.m_x, bottom_right.m_y)) {
    return;
  }
  m_top_left = top_left;
  m_bottom_right = bottom_right;
  m_x_range = m_bottom_right.m_x - m_top_left.m_x;
  m_x_axis_step = calculate_step(m_model->get_x_axis_type(), m_x_range);
  m_y_range = m_top_left.m_y - m_bottom_right.m_y;
  m_y_axis_step = calculate_step(m_model->get_y_axis_type(), m_y_range);
  update_origins();
  m_candlestick_promise = m_model->load(m_top_left.m_x,
    m_bottom_right.m_x);
  m_candlestick_promise.then([=] (auto result) {
    m_candlesticks = std::move(result.Get());
    if(m_is_auto_scaled) {
      update_auto_scale();
    } else {
      update();
    }
  });
  update();
}

void ChartView::set_auto_scale(bool auto_scale) {
  m_is_auto_scaled = auto_scale;
  if(m_is_auto_scaled) {
    update_auto_scale();
  }
}

bool ChartView::get_draw_mode() const {
  if(m_draw_state == DrawState::OFF) {
    return false;
  }
  return true;
}

void ChartView::set_draw_mode(bool draw_mode) {
  if(draw_mode) {
    m_draw_state = DrawState::IDLE;
  } else {
    m_current_trend_line_id = -1;
    m_draw_state = DrawState::OFF;
  }
  update();
}

void ChartView::set_trend_line_color(const QColor& color) {
  m_current_trend_line_color = color;
  update_selected_line_styles();
}

void ChartView::set_trend_line_style(TrendLineStyle style) {
  m_current_trend_line_style = style;
  update_selected_line_styles();
}

void ChartView::remove_selected_trend_lines() {
  if(m_draw_state == DrawState::NEW) {
    m_trend_line_model.remove(m_current_trend_line_id);
    m_draw_state = DrawState::IDLE;
  } else {
    for(auto id : m_trend_line_model.get_selected()) {
      m_trend_line_model.remove(id);
    }
  }
  update();
}

void ChartView::set_multi_select(bool on) {
  m_multi_select = on;
}

void ChartView::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.setFont(m_label_font);
  painter.setPen(Qt::white);
  painter.drawLine(m_x_origin, 0, m_x_origin, m_y_origin);
  painter.drawLine(0, m_y_origin, m_x_origin, m_y_origin);
  if(m_x_range <= ChartValue(0) || m_y_range <= ChartValue(0)) {
    return;
  }
  for(auto y : m_y_axis_values) {
    auto y_pos = map_to(y, m_bottom_right.m_y, m_top_left.m_y, m_y_origin, 0);
    painter.setPen("#3A3348");
    painter.drawLine(0, y_pos, m_x_origin, y_pos);
    painter.setPen(Qt::white);
    painter.drawLine(m_x_origin, y_pos, m_x_origin + scale_width(2), y_pos);
    painter.drawText(m_x_origin + scale_width(3),
      y_pos + (m_font_metrics.height() / 3), m_item_delegate->displayText(
      to_variant(m_model->get_y_axis_type(), y), QLocale()));
  }
  for(auto x : m_x_axis_values) {
    auto x_pos = map_to(x, m_top_left.m_x, m_bottom_right.m_x, 0,
      m_x_origin);
    painter.setPen("#3A3348");
    painter.drawLine(x_pos, 0, x_pos, m_y_origin);
    painter.setPen(Qt::white);
    painter.drawLine(x_pos, m_y_origin, x_pos, m_y_origin + scale_height(2));
    painter.drawText(x_pos - m_x_axis_text_width / 2,
      m_y_origin + m_font_metrics.height() + scale_height(2),
      m_item_delegate->displayText(to_variant(m_model->get_x_axis_type(), x),
      QLocale()));
  }
  painter.setPen(Qt::white);
  for(auto& candlestick : m_candlesticks) {
    if(candlestick.GetEnd() >= convert_pixels_to_chart({0, 0}).m_x &&
        candlestick.GetStart() <=
        convert_pixels_to_chart({m_x_origin, 0}).m_x) {
      auto open = convert_chart_to_pixels({candlestick.GetStart(),
        candlestick.GetOpen()});
      auto close = convert_chart_to_pixels({candlestick.GetEnd(),
        candlestick.GetClose()});
      auto high = map_to(candlestick.GetHigh(), m_bottom_right.m_y,
        m_top_left.m_y, m_y_origin, 0);
      auto low = map_to(candlestick.GetLow(), m_bottom_right.m_y,
        m_top_left.m_y, m_y_origin, 0);
      auto wick_pos = open.x() + (close.x() - open.x()) / 2;
      if(wick_pos < m_x_origin && high < m_y_origin) {
        painter.fillRect(QRect(QPoint(wick_pos, high),
          QPoint(wick_pos, std::min(low, m_y_origin - 1))), QColor("#C8C8C8"));
      }
      if(open.y() > close.y()) {
        if(close.y() < m_y_origin) {
          painter.fillRect(QRect(QPoint(open.x(), close.y()),
            QPoint(std::min(close.x() - 1, m_x_origin - 1),
            std::min(open.y(), m_y_origin - 1))), QColor("#8AF5C0"));
          painter.fillRect(QRect(QPoint(open.x() + 1, close.y() + 1),
            QPoint(std::min(close.x() - 2, m_x_origin - 1),
            std::min(open.y() - 1, m_y_origin - 1))), QColor("#1FD37A"));
        }
      } else {
        if(open.y() < m_y_origin) {
          painter.fillRect(QRect(open,
            QPoint(std::min(close.x() - 1, m_x_origin - 1),
            std::min(close.y(), m_y_origin - 1))), QColor("#FFA7A0"));
          painter.fillRect(QRect(QPoint(open.x() + 1, open.y() + 1),
            QPoint(std::min(close.x() - 2, m_x_origin - 1),
            std::min(close.y() - 1, m_y_origin - 1))), QColor("#EF5357"));
        }
      }
    }
  }
  if(m_crosshair_pos && m_crosshair_pos.value().x() <= m_x_origin &&
      m_crosshair_pos.value().y() <= m_y_origin) {
    if(m_draw_state == DrawState::OFF ||
        m_draw_state == DrawState::IDLE ||
        m_draw_state == DrawState::NEW) {
      setCursor(m_crosshair_cursor);
    } else {
      setCursor(m_hand_cursor);
    }
    painter.setPen(m_dashed_line_pen);
    painter.drawLine(m_crosshair_pos.value().x(), 0,
      m_crosshair_pos.value().x(), m_y_origin);
    painter.drawLine(0, m_crosshair_pos.value().y(), m_x_origin,
      m_crosshair_pos.value().y());
    auto crosshair_value = convert_pixels_to_chart(m_crosshair_pos.value());
    auto x_label = m_item_delegate->displayText(to_variant(
      m_model->get_x_axis_type(), crosshair_value.m_x), QLocale());
    auto x_label_width = m_font_metrics.width(x_label);
    painter.fillRect(m_crosshair_pos.value().x() - (x_label_width / 2) -
      scale_width(5), m_y_origin, x_label_width + scale_width(10),
      scale_height(21), Qt::white);
    painter.fillRect(m_crosshair_pos.value().x(), m_y_origin, scale_width(1),
      scale_height(3), Qt::black);
    auto text_width = m_font_metrics.width(x_label);
    painter.setPen(m_label_text_color);
    painter.drawText(m_crosshair_pos.value().x() - text_width / 2,
      m_y_origin + m_font_metrics.height() + scale_height(2), x_label);
    painter.fillRect(m_x_origin,
      m_crosshair_pos.value().y() - (scale_height(15) / 2),
      width() - m_x_origin, scale_height(15), Qt::white);
    painter.fillRect(m_x_origin, m_crosshair_pos.value().y(), scale_width(3),
      scale_height(1), Qt::black);
    auto y_label = m_item_delegate->displayText(to_variant(
      m_model->get_y_axis_type(), crosshair_value.m_y), QLocale());
    painter.setPen(m_label_text_color);
    painter.drawText(m_x_origin + scale_width(3), m_crosshair_pos.value().y() +
      (m_font_metrics.height() / 3), y_label);
  } else {
    setCursor(Qt::ArrowCursor);
  }
  painter.setClipRegion({0, 0, m_x_origin, m_y_origin});
  for(auto& line : m_trend_line_model.get_lines()) {
    auto first = convert_chart_to_pixels(std::get<0>(line.m_points));
    auto second = convert_chart_to_pixels(std::get<1>(line.m_points));
    draw_trend_line(painter, line.m_style, line.m_color, first.x(), first.y(),
      second.x(), second.y());
  }
  if(m_draw_state != DrawState::OFF) {
    for(auto id : m_trend_line_model.get_selected()) {
      draw_points(id, painter);
    }
    if(m_draw_state == DrawState::HOVER ||
        m_draw_state == DrawState::LINE ||
        m_draw_state == DrawState::POINT) {
      draw_points(m_current_trend_line_id, painter);
    }
  }
}

void ChartView::resizeEvent(QResizeEvent* event) {
  update_origins();
  QWidget::resizeEvent(event);
}

void ChartView::showEvent(QShowEvent* event) {
  if(m_top_left.m_x == ChartValue() && m_top_left.m_y == ChartValue() &&
      m_bottom_right.m_x == ChartValue() &&
      m_bottom_right.m_y == ChartValue()) {
    auto current_time = boost::posix_time::second_clock::local_time();
    auto bottom_right = ChartPoint(ChartValue(current_time),
      ChartValue(Nexus::Money(0)));
    auto top_left = ChartPoint(
      ChartValue(current_time - boost::posix_time::hours(1)),
      ChartValue(Nexus::Money(1)));
    set_region(top_left, bottom_right);
  }
  QWidget::showEvent(event);
}

ChartPoint ChartView::chart_delta(const QPoint& previous,
    const QPoint& present) {
  auto previous_value = convert_pixels_to_chart(previous);
  auto present_value = convert_pixels_to_chart(present);
  return {previous_value.m_x - present_value.m_x,
    previous_value.m_y - present_value.m_y};
}

void ChartView::clear_selections() {
  for(auto id : m_trend_line_model.get_selected()) {
    m_trend_line_model.unset_selected(id);
  }
}

void ChartView::draw_points(int id, QPainter& painter) {
  auto line = m_trend_line_model.get(id);
  auto first = convert_chart_to_pixels(std::get<0>(line.m_points));
  auto second = convert_chart_to_pixels(std::get<1>(line.m_points));
  painter.setPen(QColor("#25212E"));
  painter.setBrush(QColor("#25212E"));
  painter.drawEllipse(first, scale_width(5), scale_height(5));
  painter.drawEllipse(second, scale_width(5), scale_height(5));
  painter.setPen(Qt::white);
  painter.setBrush(Qt::white);
  painter.drawEllipse(first, scale_width(4), scale_height(4));
  painter.drawEllipse(second, scale_width(4), scale_height(4));
}

void ChartView::invert_selection(int id) {
  if(m_trend_line_model.is_selected(id)) {
    m_trend_line_model.unset_selected(id);
  } else {
    m_trend_line_model.set_selected(id);
  }
}

void ChartView::update_auto_scale() {
  if(m_candlesticks.empty()) {
    return;
  }
  auto auto_scale_top = m_candlesticks.front().GetHigh();
  auto auto_scale_bottom = m_candlesticks.front().GetLow();
  for(auto& candle : m_candlesticks) {
    auto_scale_top = std::max(auto_scale_top, candle.GetHigh());
    auto_scale_bottom = std::min(auto_scale_bottom, candle.GetLow());
  }
  set_region({m_top_left.m_x, auto_scale_top},
    {m_bottom_right.m_x, auto_scale_bottom});
}

int ChartView::update_intersection(const QPoint& mouse_pos) {
  auto id = m_trend_line_model.find_closest(
    convert_pixels_to_chart(*m_crosshair_pos));
  if(id == -1) {
    return id;
  }
  auto mouse_x = static_cast<double>(mouse_pos.x());
  auto mouse_y = static_cast<double>(mouse_pos.y());
  auto line = m_trend_line_model.get(id);
  auto point1 = convert_chart_to_pixels(std::get<0>(line.m_points));
  auto point2 = convert_chart_to_pixels(std::get<1>(line.m_points));
  auto point1_x = static_cast<double>(point1.x());
  auto point1_y = static_cast<double>(point1.y());
  auto point2_x = static_cast<double>(point2.x());
  auto point2_y = static_cast<double>(point2.y());
  auto line_slope = slope(point1_x, point1_y, point2_x, point2_y);
  auto line_b = y_intercept(point1_x, point1_y, line_slope);
  auto point_distance = std::abs(closest_point_distance_squared(mouse_x,
    mouse_y, point1_x, point1_y, point2_x, point2_y));
  auto distance = std::numeric_limits<double>::infinity();
  if(std::isinf<double>(line_slope)) {
    if(is_within_interval(mouse_y, point1_y, point2_y)) {
      distance = std::abs(distance_squared(mouse_x, mouse_y,
        point1_x, mouse_y));
    }
  } else if(line_slope == Quantity(0)) {
    if(is_within_interval(mouse_x, point1_x, point2_x)) {
      distance = std::abs(distance_squared(mouse_x, mouse_y, mouse_x,
        point1_y));
    }
  } else {
    auto line_point_x =
      (mouse_x + line_slope * mouse_y - line_slope * line_b) /
      (line_slope * line_slope + 1);
    if(is_within_interval(line_point_x, point1_x, point2_x)) {
      distance = std::abs(distance_squared(mouse_x, mouse_y, line_point_x,
        calculate_y(line_slope, line_point_x, line_b)));
    }
  }
  if(point_distance <= LINE_HOVER_DISTANCE ||
      distance <= LINE_HOVER_DISTANCE) {
    return id;
  }
  return -1;
}

void ChartView::update_origins() {
  auto x_value = m_top_left.m_x - (m_top_left.m_x % m_x_axis_step) +
    m_x_axis_step;
  m_x_axis_values.clear();
  m_x_axis_text_width = 0;
  while(x_value <= m_bottom_right.m_x) {
    m_x_axis_values.push_back(x_value);
    auto text_width = m_font_metrics.width(m_item_delegate->displayText(
      to_variant(m_model->get_x_axis_type(), x_value), QLocale()));
    m_x_axis_text_width = std::max(m_x_axis_text_width, text_width);
    x_value += m_x_axis_step;
  }
  auto y_value = m_bottom_right.m_y - (m_bottom_right.m_y % m_y_axis_step) +
    m_y_axis_step;
  m_y_axis_values.clear();
  auto old_x_origin = m_x_origin;
  m_x_origin = INT_MAX;
  auto top_label = m_top_left.m_y - (m_top_left.m_y % m_y_axis_step);
  while(y_value <= top_label) {
    m_y_axis_values.push_back(y_value);
    auto origin = width() - (m_font_metrics.width("M") * (
      m_item_delegate->displayText(to_variant(m_model->get_y_axis_type(),
      y_value), QLocale()).length()) - scale_width(4));
    m_x_origin = std::min(m_x_origin, origin);
    y_value += m_y_axis_step;
  }
  if(m_x_origin == INT_MAX) {
    m_x_origin = old_x_origin;
  }
  m_y_origin = height() - (m_font_metrics.height() + scale_height(9));
}

void ChartView::update_selected_line_styles() {
  for(auto id : m_trend_line_model.get_selected()) {
    auto line = m_trend_line_model.get(id);
    m_trend_line_model.update(TrendLine{line.m_points,
      m_current_trend_line_color, m_current_trend_line_style}, id);
  }
  update();
}

void ChartView::on_left_mouse_button_press(const QPoint& pos) {
  if(m_draw_state == DrawState::HOVER) {
    if(m_multi_select) {
      invert_selection(m_current_trend_line_id);
      return;
    }
    clear_selections();
    m_trend_line_model.set_selected(m_current_trend_line_id);
    auto line = m_trend_line_model.get(m_current_trend_line_id);
    auto point1 = convert_chart_to_pixels(std::get<0>(line.m_points));
    auto point2 = convert_chart_to_pixels(std::get<1>(line.m_points));
    auto point1_x = static_cast<double>(point1.x());
    auto point1_y = static_cast<double>(point1.y());
    auto point2_x = static_cast<double>(point2.x());
    auto point2_y = static_cast<double>(point2.y());
    auto point_distance = std::abs(closest_point_distance_squared(
      static_cast<double>(pos.x()), static_cast<double>(pos.y()),
      point1_x, point1_y, point2_x, point2_y));
    if(point_distance < LINE_HOVER_DISTANCE) {
      auto line = m_trend_line_model.get(m_current_trend_line_id);
      auto line_point = convert_chart_to_pixels(std::get<0>(line.m_points));
      auto distance = distance_squared(pos.x(), pos.y(), line_point.x(),
        line_point.y());
      if(point_distance == distance) {
        m_current_trend_line_point = std::get<0>(line.m_points);
        m_current_stationary_point = std::get<1>(line.m_points);
      } else {
        m_current_trend_line_point = std::get<1>(line.m_points);
        m_current_stationary_point = std::get<0>(line.m_points);
      }
      m_trend_line_model.set_selected(m_current_trend_line_id);
      m_draw_state = DrawState::POINT;
    } else {
      m_trend_line_model.set_selected(m_current_trend_line_id);
      m_draw_state = DrawState::LINE;
    }
  } else if(m_draw_state == DrawState::IDLE) {
    if(m_trend_line_model.get_selected().empty()) {
      m_current_trend_line_point = convert_pixels_to_chart(pos);
      m_current_stationary_point = m_current_trend_line_point;
      m_current_trend_line_id = m_trend_line_model.add(
        TrendLine({m_current_trend_line_point, m_current_trend_line_point},
        m_current_trend_line_color,
        m_current_trend_line_style));
      m_draw_state = DrawState::NEW;
    } else {
      clear_selections();
      m_draw_state = DrawState::IDLE;
    }
  } else if(m_draw_state == DrawState::NEW) {
    auto line = m_trend_line_model.get(m_current_trend_line_id);
    m_current_trend_line_point = convert_pixels_to_chart(pos);
    m_trend_line_model.update(TrendLine{{m_current_trend_line_point,
      m_current_stationary_point}, line.m_color, line.m_style},
      m_current_trend_line_id);
    m_draw_state = DrawState::IDLE;
  }
}

void ChartView::on_left_mouse_button_release() {
  if(m_draw_state == DrawState::LINE || m_draw_state == DrawState::POINT) {
    m_draw_state = DrawState::IDLE;
  }
}

void ChartView::on_right_mouse_button_press() {
  if(m_draw_state == DrawState::HOVER || m_draw_state == DrawState::NEW) {
    m_trend_line_model.remove(m_current_trend_line_id);
    m_current_trend_line_id = -1;
    m_draw_state = DrawState::IDLE;
  }
}
