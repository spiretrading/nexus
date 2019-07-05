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
    return static_cast<U>((value - a) / (b - a) * (d - c) + c);
  }

  template<typename U>
  U map_to(int value, int a, int b, U c, U d) {
    return map_to(static_cast<double>(value), static_cast<double>(a),
      static_cast<double>(b), c, d);
  }

  template<typename T>
  int map_to(T value, T a, T b, int c, int d) {
    return static_cast<int>(
      map_to(value, a, b, static_cast<double>(c), static_cast<double>(d)));
  }

  ChartValue calculate_step(ChartValue::Type value_type, ChartValue range) {
    if(value_type == ChartValue::Type::MONEY) {
      return ChartValue(Money::ONE);
    } else if(value_type == ChartValue::Type::TIMESTAMP) {
      return ChartValue(minutes(10));
    }
    return ChartValue();
  }

  const auto& HAND_CURSOR() {
    static auto cursor = QCursor(QPixmap::fromImage(
      imageFromSvg(":/Icons/finger-cursor.svg", scale(18, 18))), 0, 0);
    return cursor;
  }

  const auto& CROSSHAIR_CURSOR() {
    static auto cursor = QCursor(QPixmap::fromImage(
      imageFromSvg(":/Icons/chart-cursor.svg", scale(18, 18))));
    return cursor;
  }

  const auto& GAP_SLASH_IMAGE() {
    static auto cursor = imageFromSvg(":/Icons/slash-texture.svg", scale(4, 3));
    return cursor;
  }

  const auto GAP_SIZE() {
    static auto size = scale_width(35);
    return size;
  }
}

ChartView::ChartView(ChartModel& model, QWidget* parent)
    : QWidget(parent),
      m_model(&model),
      m_label_font("Roboto"),
      m_font_metrics(m_label_font),
      m_item_delegate(new CustomVariantItemDelegate(this)),
      m_dashed_line_pen(QColor("#E5E5E5"), scale_width(1), Qt::CustomDashLine),
      m_label_text_color(QColor("#25212E")),
      m_is_auto_scaled(true),
      m_draw_state(DrawState::OFF),
      m_mouse_buttons(Qt::NoButton),
      m_line_hover_distance_squared(scale_width(6) * scale_width(6)),
      m_is_multi_select_enabled(false) {
  setFocusPolicy(Qt::NoFocus);
  setMouseTracking(true);
  setAttribute(Qt::WA_Hover);
  m_label_font.setPixelSize(scale_height(10));
  m_font_metrics = QFontMetrics(m_label_font);
  m_dashed_line_pen.setDashPattern({static_cast<double>(scale_width(3)),
    static_cast<double>(scale_width(3))});
}

ChartPoint ChartView::to_chart_point(const QPoint& point) const {
  auto y = map_to(point.y(), m_bottom_right_pixel.y(), 0, m_bottom_right.m_y,
    m_top_left.m_y);
  auto lower_x_pixel = 0;
  auto lower_x_chart_value = m_top_left.m_x;
  auto x = [&] {
    for(auto& gap : m_gaps) {
      auto gap_start_pixel = to_pixel({gap.m_start, y}).x();
      if(point.x() <= gap_start_pixel) {
        return map_to(point.x(), lower_x_pixel, gap_start_pixel,
          lower_x_chart_value, gap.m_start);
      }
      auto gap_end_pixel = to_pixel({gap.m_end, y}).x();
      if(point.x() < gap_end_pixel) {
        return map_to(point.x(), gap_start_pixel, gap_end_pixel,
          gap.m_start, gap.m_end);
      }
      lower_x_pixel = gap_end_pixel;
      lower_x_chart_value = gap.m_end;
    }
    return map_to(point.x(), lower_x_pixel, m_bottom_right_pixel.x(),
      lower_x_chart_value, m_bottom_right.m_x);
  }();
  return {x, y};
}

QPoint ChartView::to_pixel(const ChartPoint& point) const {
  auto x = map_to(point.m_x, m_top_left.m_x, m_bottom_right.m_x, 0,
    m_bottom_right_pixel.x());
  for(auto& gap : m_gaps) {
    if(gap.m_start < point.m_x && gap.m_end > point.m_x) {
      auto new_x = to_pixel({gap.m_start, ChartValue()}).x() +
        static_cast<int>((point.m_x - gap.m_start) /
        (gap.m_end - gap.m_start) * static_cast<double>(GAP_SIZE()));
      return {new_x, map_to(point.m_y, m_bottom_right.m_y, m_top_left.m_y,
        m_bottom_right_pixel.y(), 0)};
    }
    if(point.m_x > gap.m_start) {
      auto gap_start = map_to(gap.m_start, m_top_left.m_x, m_bottom_right.m_x,
        0, m_bottom_right_pixel.x());
      auto gap_end = map_to(gap.m_end, m_top_left.m_x, m_bottom_right.m_x,
        0, m_bottom_right_pixel.x());
      x -= gap_end - gap_start - GAP_SIZE();
    }
  }
  return {x, map_to(point.m_y, m_bottom_right.m_y, m_top_left.m_y,
    m_bottom_right_pixel.y(), 0)};
}

void ChartView::set_crosshair(const ChartPoint& position,
    Qt::MouseButtons buttons) {
  set_crosshair(to_pixel(position), buttons);
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
      auto delta = *m_crosshair_pos - m_last_crosshair_pos;
      auto first = to_chart_point(to_pixel(std::get<0>(line.m_points)) +
        delta);
      auto chart_delta = first - std::get<0>(line.m_points);
      line.m_points = {first, std::get<1>(line.m_points) + chart_delta};
      m_trend_line_model.update(line, m_current_trend_line_id);
    } else if(m_draw_state == DrawState::NEW) {
      auto line = m_trend_line_model.get(m_current_trend_line_id);
      m_current_trend_line_point = to_chart_point(*m_crosshair_pos);
      m_trend_line_model.update(TrendLine{{m_current_trend_line_point,
        m_current_stationary_point}, line.m_color, line.m_style},
        m_current_trend_line_id);
    } else if(m_draw_state == DrawState::POINT) {
      auto line = m_trend_line_model.get(m_current_trend_line_id);
      m_current_trend_line_point = to_chart_point(*m_crosshair_pos);
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
  if(top_left == m_top_left && bottom_right == m_bottom_right) {
    return;
  }
  m_top_left = top_left;
  m_bottom_right = bottom_right;
  m_x_range = m_bottom_right.m_x - m_top_left.m_x;
  m_x_axis_step = calculate_step(m_model->get_x_axis_type(), m_x_range);
  m_y_range = m_top_left.m_y - m_bottom_right.m_y;
  m_y_axis_step = calculate_step(m_model->get_y_axis_type(), m_y_range);
  update_origins();
  m_candlestick_promise = m_model->load(m_top_left.m_x, m_bottom_right.m_x);
  m_candlestick_promise.then([=] (auto result) {
    m_candlesticks = std::move(result.Get());
    update_gaps();
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

bool ChartView::is_draw_mode_enabled() const {
  return m_draw_state != DrawState::OFF;
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
  } else {
    for(auto id : m_trend_line_model.get_selected()) {
      m_trend_line_model.remove(id);
    }
  }
  m_draw_state = DrawState::IDLE;
  update();
}

void ChartView::set_multi_select(bool on) {
  m_is_multi_select_enabled = on;
}

void ChartView::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.setFont(m_label_font);
  painter.setPen(Qt::white);
  painter.drawLine(m_bottom_right_pixel.x(), 0, m_bottom_right_pixel.x(),
    m_bottom_right_pixel.y());
  painter.drawLine(0, m_bottom_right_pixel.y(), m_bottom_right_pixel.x(),
    m_bottom_right_pixel.y());
  if(m_x_range <= ChartValue(0) || m_y_range <= ChartValue(0)) {
    return;
  }
  for(auto y : m_y_axis_values) {
    auto y_pos = map_to(y, m_bottom_right.m_y, m_top_left.m_y,
      m_bottom_right_pixel.y(), 0);
    painter.setPen("#3A3348");
    painter.drawLine(0, y_pos, m_bottom_right_pixel.x(), y_pos);
    painter.setPen(Qt::white);
    painter.drawLine(m_bottom_right_pixel.x(), y_pos,
      m_bottom_right_pixel.x() + scale_width(2), y_pos);
    painter.drawText(m_bottom_right_pixel.x() + scale_width(3),
      y_pos + (m_font_metrics.height() / 3), m_item_delegate->displayText(
      to_variant(m_model->get_y_axis_type(), y), QLocale()));
  }
  for(auto x : m_x_axis_values) {
    auto x_pos = static_cast<int>(to_pixel({x, ChartValue()}).x());
    if(x_pos < m_bottom_right_pixel.x() && !intersects_gap(x_pos)) {
      painter.setPen("#3A3348");
      painter.drawLine(x_pos, 0, x_pos, m_bottom_right_pixel.y());
      painter.setPen(Qt::white);
      painter.drawLine(x_pos, m_bottom_right_pixel.y(), x_pos,
        m_bottom_right_pixel.y() + scale_height(2));
      auto text_width = m_font_metrics.width(m_item_delegate->displayText(
        to_variant(m_model->get_x_axis_type(), x), QLocale()));
      painter.drawText(x_pos - text_width / 2,
        m_bottom_right_pixel.y() + m_font_metrics.height() + scale_height(2),
        m_item_delegate->displayText(to_variant(m_model->get_x_axis_type(), x),
        QLocale()));
    }
  }
  painter.setPen(Qt::white);
  for(auto& candlestick : m_candlesticks) {
    auto open = to_pixel({candlestick.GetStart(), candlestick.GetOpen()});
    auto close = to_pixel({candlestick.GetEnd(), candlestick.GetClose()});
    const auto GAP_DIVISOR = 2.95;
    auto start_x = static_cast<int>((open.x() - (close.x() - open.x()) /
      GAP_DIVISOR) + 1);
    auto end_x = static_cast<int>(open.x() + (close.x() - open.x()) /
      GAP_DIVISOR);
    if(candlestick.GetEnd() >= m_top_left.m_x && start_x <=
        m_bottom_right_pixel.x()) {
      auto high = map_to(candlestick.GetHigh(), m_bottom_right.m_y,
        m_top_left.m_y, m_bottom_right_pixel.y(), 0);
      auto low = map_to(candlestick.GetLow(), m_bottom_right.m_y,
        m_top_left.m_y, m_bottom_right_pixel.y(), 0);
      if(open.x() < m_bottom_right_pixel.x() && high <
          m_bottom_right_pixel.y()) {
        painter.fillRect(QRect(QPoint(open.x(), high),
          QPoint(open.x(), std::min(low, m_bottom_right_pixel.y() - 1))),
          QColor("#A0A0A0"));
      }
      if(open.y() > close.y() && close.y() < m_bottom_right_pixel.y()) {
        painter.fillRect(QRect(QPoint(start_x, close.y()),
          QPoint(std::min(end_x - 1, m_bottom_right_pixel.x() - 1),
          std::min(open.y(), m_bottom_right_pixel.y() - 1))),
          QColor("#8AF5C0"));
        painter.fillRect(QRect(QPoint(start_x + 1, close.y() + 1),
          QPoint(std::min(end_x - 2, m_bottom_right_pixel.x() - 1),
          std::min(open.y() - 1, m_bottom_right_pixel.y() - 1))),
          QColor("#1FD37A"));
      } else if(open.y() < m_bottom_right_pixel.y()) {
        painter.fillRect(QRect({start_x, open.y()},
          QPoint(std::min(end_x - 1, m_bottom_right_pixel.x() - 1),
          std::min(close.y(), m_bottom_right_pixel.y() - 1))),
          QColor("#FFA7A0"));
        painter.fillRect(QRect(QPoint(start_x + 1, open.y() + 1),
          QPoint(std::min(end_x - 2, m_bottom_right_pixel.x() - 1),
          std::min(close.y() - 1, m_bottom_right_pixel.y() - 1))),
          QColor("#EF5357"));
      }
    }
  }
  for(auto& gap : m_gaps) {
    auto start = to_pixel({gap.m_start, ChartValue()}).x();
    if(start < m_bottom_right_pixel.x()) {
      draw_gap(painter, start, to_pixel({gap.m_end, ChartValue()}).x());
    }
  }
  if(m_crosshair_pos && m_crosshair_pos.value().x() <=
      m_bottom_right_pixel.x() && m_crosshair_pos.value().y() <=
      m_bottom_right_pixel.y()) {
    if(m_draw_state == DrawState::OFF ||
        m_draw_state == DrawState::IDLE ||
        m_draw_state == DrawState::NEW) {
      setCursor(CROSSHAIR_CURSOR());
    } else {
      setCursor(HAND_CURSOR());
    }
    painter.setPen(m_dashed_line_pen);
    painter.drawLine(m_crosshair_pos.value().x(), 0,
      m_crosshair_pos.value().x(), m_bottom_right_pixel.y());
    painter.drawLine(0, m_crosshair_pos.value().y(), m_bottom_right_pixel.x(),
      m_crosshair_pos.value().y());
    auto crosshair_value = to_chart_point(m_crosshair_pos.value());
    if(m_gaps.empty() || !intersects_gap(m_crosshair_pos->x())) {
      auto x_label = m_item_delegate->displayText(to_variant(
        m_model->get_x_axis_type(), crosshair_value.m_x), QLocale());
      auto x_label_width = m_font_metrics.width(x_label);
      painter.fillRect(m_crosshair_pos.value().x() - (x_label_width / 2) -
        scale_width(5), m_bottom_right_pixel.y(), x_label_width +
        scale_width(10), scale_height(21), Qt::white);
      painter.fillRect(m_crosshair_pos.value().x(), m_bottom_right_pixel.y(),
        scale_width(1), scale_height(3), Qt::black);
      auto text_width = m_font_metrics.width(x_label);
      painter.setPen(m_label_text_color);
      painter.drawText(m_crosshair_pos.value().x() - text_width / 2,
        m_bottom_right_pixel.y() + m_font_metrics.height() + scale_height(2),
        x_label);
    } else {
      painter.fillRect(m_crosshair_pos.value().x() - (scale_width(64) / 2),
        m_bottom_right_pixel.y(), scale_width(64), scale_height(21),
        Qt::white);
      painter.fillRect(m_crosshair_pos.value().x(), m_bottom_right_pixel.y(),
        scale_width(1), scale_height(3), Qt::black);
      auto text_width = m_font_metrics.width(tr("No Activity"));
      painter.setPen(m_label_text_color);
      painter.drawText(m_crosshair_pos.value().x() - text_width / 2,
        m_bottom_right_pixel.y() + m_font_metrics.height() + scale_height(2),
        tr("No Activity"));
    }
    painter.fillRect(m_bottom_right_pixel.x(),
      m_crosshair_pos.value().y() - (scale_height(15) / 2),
      width() - m_bottom_right_pixel.x(), scale_height(15), Qt::white);
    painter.fillRect(m_bottom_right_pixel.x(), m_crosshair_pos.value().y(),
      scale_width(3), scale_height(1), Qt::black);
    auto y_label = m_item_delegate->displayText(to_variant(
      m_model->get_y_axis_type(), crosshair_value.m_y), QLocale());
    painter.setPen(m_label_text_color);
    painter.drawText(m_bottom_right_pixel.x() + scale_width(3),
      m_crosshair_pos.value().y() + (m_font_metrics.height() / 3), y_label);
  } else {
    setCursor(Qt::ArrowCursor);
  }
  painter.setClipRegion({0, 0, m_bottom_right_pixel.x(),
    m_bottom_right_pixel.y()});
  for(auto& line : m_trend_line_model.get_lines()) {
    auto first = to_pixel(std::get<0>(line.m_points));
    auto second = to_pixel(std::get<1>(line.m_points));
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
    //auto current_time = boost::posix_time::second_clock::local_time();
    auto current_time = Nexus::Money(100);
    auto bottom_right = ChartPoint(ChartValue(current_time),
      ChartValue(Nexus::Money(0)));
    auto top_left = ChartPoint(
      //ChartValue(current_time - boost::posix_time::hours(1)),
      ChartValue(Nexus::Money(0)),
      ChartValue(Nexus::Money(1)));
    set_region(top_left, bottom_right);
  }
  QWidget::showEvent(event);
}

void ChartView::draw_gap(QPainter& painter, int start, int end) {
  painter.fillRect(start, m_bottom_right_pixel.y(), end - start,
    scale_height(3), QColor("#25212E"));
  painter.save();
  painter.setPen(Qt::white);
  painter.drawLine(start, m_bottom_right_pixel.y(), start,
    m_bottom_right_pixel.y() +  scale_height(2));
  if(end <= m_bottom_right_pixel.x()) {
    painter.drawLine(end, m_bottom_right_pixel.y(), end,
      m_bottom_right_pixel.y() + scale_height(2));
  }
  end = std::min(end, m_bottom_right_pixel.x());
  painter.setPen("#8C8C8C");
  auto slash_count = (static_cast<double>(end) - static_cast<double>(start)) /
    (static_cast<double>(scale_width(4)) +
    static_cast<double>(scale_width(1))) - 1.0;
  auto padding = std::fmod(slash_count, scale_width(4) + scale_width(1)) / 2;
  auto x = start + static_cast<int>(padding) + scale_width(1);
  for(auto i = 0; i < slash_count; ++i) {
    painter.drawImage(x, m_bottom_right_pixel.y(), GAP_SLASH_IMAGE());
    x += scale_width(4) + scale_width(1);
  }
  painter.restore();
}

void ChartView::draw_point(QPainter& painter, const QColor& border_color,
    const QPoint& pos) {
  painter.setPen(border_color);
  painter.setBrush(border_color);
  painter.drawEllipse(pos, scale_width(6), scale_height(6));
  painter.setPen(Qt::white);
  painter.setBrush(Qt::white);
  painter.drawEllipse(pos, scale_width(4), scale_height(4));
}

void ChartView::draw_points(int id, QPainter& painter) {
  auto line = m_trend_line_model.get(id);
  auto first = to_pixel(std::get<0>(line.m_points));
  auto second = to_pixel(std::get<1>(line.m_points));
  auto current_point = to_pixel(m_current_trend_line_point);
  auto first_color = QColor("#25212E");
  auto second_color = QColor("#25212E");
  if(current_point == first) {
    first_color = QColor("#B9B4EC");
  } else if(current_point == second) {
    second_color = QColor("#B9B4EC");
  }
  draw_point(painter, first_color, first);
  draw_point(painter, second_color, second);
}

bool ChartView::intersects_gap(int x) const {
  if(m_gaps.empty()) {
    return false;
  }
  auto chart_x = to_chart_point({x, 0}).m_x;
  return m_gaps.end() != std::find_if(m_gaps.begin(), m_gaps.end(),
    [&] (const auto& gap) {
      return gap.m_start < chart_x && gap.m_end > chart_x;
    });
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

void ChartView::update_gaps() {
  if(m_candlesticks.empty()) {
    return;
  }
  m_gaps.clear();
  for(auto i = m_candlesticks.begin(); i < m_candlesticks.end() - 1; ++i) {
    auto end = i->GetEnd();
    auto next_start = std::next(i)->GetStart();
    if(end != next_start) {
      m_gaps.push_back({end, next_start});
    }
  }
}

int ChartView::update_intersection(const QPoint& mouse_pos) {
  auto id = m_trend_line_model.find_closest(
    to_chart_point(*m_crosshair_pos));
  if(id == -1) {
    return id;
  }
  auto line = m_trend_line_model.get(id);
  auto point1 = to_pixel(std::get<0>(line.m_points));
  auto point2 = to_pixel(std::get<1>(line.m_points));
  auto line_slope = slope(point1, point2);
  auto line_b = y_intercept(point1, line_slope);
  auto point_distance = closest_point_distance_squared(mouse_pos, point1,
    point2);
  if(point_distance < m_line_hover_distance_squared) {
    auto line = m_trend_line_model.get(id);
    auto line_point = to_pixel(std::get<0>(line.m_points));
    auto distance = distance_squared(mouse_pos, point1);
    if(point_distance == distance) {
      m_current_trend_line_point = std::get<0>(line.m_points);
      m_current_stationary_point = std::get<1>(line.m_points);
    } else {
      m_current_trend_line_point = std::get<1>(line.m_points);
      m_current_stationary_point = std::get<0>(line.m_points);
    }
  } else {
    m_current_trend_line_point = ChartPoint();
    m_current_stationary_point = ChartPoint();
  }
  auto distance = std::numeric_limits<double>::infinity();
  if(std::isinf<double>(line_slope)) {
    if(is_within_interval(mouse_pos.y(), point1.y(), point2.y())) {
      distance = distance_squared(mouse_pos,
        {static_cast<double>(point1.x()), static_cast<double>(mouse_pos.y())});
    }
  } else if(line_slope == 0) {
    if(is_within_interval(mouse_pos.x(), point1.x(), point2.x())) {
      distance = distance_squared(mouse_pos,
        {static_cast<double>(mouse_pos.x()), static_cast<double>(point1.y())});
    }
  } else {
    auto line_point_x =
      (mouse_pos.x() + line_slope * mouse_pos.y() - line_slope * line_b) /
      (line_slope * line_slope + 1);
    if(is_within_interval(line_point_x, point1.x(), point2.x())) {
      distance = distance_squared(mouse_pos, {line_point_x,
        calculate_y(line_slope, line_point_x, line_b)});
    }
  }
  if(point_distance <= m_line_hover_distance_squared ||
      distance <= m_line_hover_distance_squared) {
    return id;
  }
  return -1;
}

void ChartView::update_origins() {
  auto x_value = m_top_left.m_x - (m_top_left.m_x % m_x_axis_step) +
    m_x_axis_step;
  m_x_axis_values.clear();
  while(x_value <= m_bottom_right.m_x) {
    for(auto& gap : m_gaps) {
      if(gap.m_start < x_value && x_value < gap.m_end) {
        x_value = gap.m_end;
        break;
      }
    }
    m_x_axis_values.push_back(x_value);
    x_value += m_x_axis_step;
  }
  auto y_value = m_bottom_right.m_y - (m_bottom_right.m_y % m_y_axis_step) +
    m_y_axis_step;
  m_y_axis_values.clear();
  auto old_x_origin = m_bottom_right_pixel.x();
  m_bottom_right_pixel.setX(INT_MAX);
  auto top_label = m_top_left.m_y - (m_top_left.m_y % m_y_axis_step);
  while(y_value <= top_label) {
    m_y_axis_values.push_back(y_value);
    auto origin = width() - (m_font_metrics.width("M") * (
      m_item_delegate->displayText(to_variant(m_model->get_y_axis_type(),
      y_value), QLocale()).length()) - scale_width(4));
    m_bottom_right_pixel.setX(std::min(m_bottom_right_pixel.x(), origin));
    y_value += m_y_axis_step;
  }
  if(m_bottom_right_pixel.x() == INT_MAX) {
    m_bottom_right_pixel.setX(old_x_origin);
  }
  m_bottom_right_pixel.setY(height() - (m_font_metrics.height() +
    scale_height(9)));
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
    if(m_is_multi_select_enabled) {
      m_trend_line_model.toggle_selection(m_current_trend_line_id);
      return;
    }
    m_trend_line_model.clear_selected();
    m_trend_line_model.set_selected(m_current_trend_line_id);
    if(m_current_trend_line_point.m_x != ChartValue() &&
        m_current_trend_line_point.m_y != ChartValue()) {
      m_trend_line_model.set_selected(m_current_trend_line_id);
      m_draw_state = DrawState::POINT;
    } else {
      m_trend_line_model.set_selected(m_current_trend_line_id);
      m_draw_state = DrawState::LINE;
    }
  } else if(m_draw_state == DrawState::IDLE) {
    if(m_trend_line_model.get_selected().empty()) {
      m_current_trend_line_point = to_chart_point(pos);
      m_current_stationary_point = m_current_trend_line_point;
      m_current_trend_line_id = m_trend_line_model.add(
        TrendLine({m_current_trend_line_point, m_current_trend_line_point},
        m_current_trend_line_color,
        m_current_trend_line_style));
      m_draw_state = DrawState::NEW;
    } else {
      m_trend_line_model.clear_selected();
      m_draw_state = DrawState::IDLE;
    }
  } else if(m_draw_state == DrawState::NEW) {
    auto line = m_trend_line_model.get(m_current_trend_line_id);
    m_current_trend_line_point = to_chart_point(pos);
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
