#include "Spire/Charting/ChartView.hpp"
#include <locale>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include "Nexus/Definitions/Money.hpp"
#include "Spire/Charting/ChartModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Beam::Queries;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  QVariant to_variant(Scalar::Type type, Scalar value) {
    if(type == Scalar::Type::DURATION) {
      return QVariant::fromValue(static_cast<time_duration>(value));
    } else if(type == Scalar::Type::MONEY) {
      return QVariant::fromValue(static_cast<Money>(value));
    } else if(type == Scalar::Type::QUANTITY) {
      return QVariant::fromValue(static_cast<Quantity>(value));
    } else if(type == Scalar::Type::TIMESTAMP) {
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

  Scalar calculate_step(Scalar::Type type, Scalar range) {
    return range / 10;
  }

  const auto GAP_SIZE() {
    static auto size = scale_width(35);
    return size;
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
}

bool ChartView::Region::operator ==(const Region& rhs) const {
  return m_top_left == rhs.m_top_left &&
    m_bottom_right == rhs.m_bottom_right;
}

bool ChartView::Region::operator !=(const Region& rhs) const {
  return !(*this == rhs);
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
      m_time_per_point(1000.),
      m_draw_state(DrawState::OFF),
      m_mouse_buttons(Qt::NoButton),
      m_line_hover_distance_squared(scale_width(6) * scale_width(6)),
      m_is_multi_select_enabled(false),
      m_data_update_promise([] { return std::nullopt; }) {
  setFocusPolicy(Qt::NoFocus);
  setMouseTracking(true);
  setAttribute(Qt::WA_Hover);
  m_label_font.setPixelSize(scale_height(10));
  m_font_metrics = QFontMetrics(m_label_font);
  m_dashed_line_pen.setDashPattern({static_cast<double>(scale_width(3)),
    static_cast<double>(scale_width(3))});
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
      if(!m_line_mouse_offset) {
        m_line_mouse_offset = LineMouseOffset{
          *to_pixel(std::get<0>(line.m_points)) - *m_crosshair_pos,
          *to_pixel(std::get<1>(line.m_points)) - *m_crosshair_pos};
      } else {
        line.m_points = {
          *to_chart_point(*m_crosshair_pos + m_line_mouse_offset->m_first),
          *to_chart_point(*m_crosshair_pos + m_line_mouse_offset->m_second)};
        m_trend_line_model.update(line, m_current_trend_line_id);
      }
    } else if(m_draw_state == DrawState::NEW) {
      auto line = m_trend_line_model.get(m_current_trend_line_id);
      m_current_trend_line_point = *to_chart_point(*m_crosshair_pos);
      m_trend_line_model.update(TrendLine{{m_current_trend_line_point,
        m_current_stationary_point}, line.m_color, line.m_style},
        m_current_trend_line_id);
    } else if(m_draw_state == DrawState::POINT) {
      auto line = m_trend_line_model.get(m_current_trend_line_id);
      m_current_trend_line_point = *to_chart_point(*m_crosshair_pos);
      m_trend_line_model.update(TrendLine{{m_current_trend_line_point,
        m_current_stationary_point}, line.m_color, line.m_style},
        m_current_trend_line_id);
    }
  }
  update();
}

void ChartView::reset_crosshair() {
  m_crosshair_pos = std::nullopt;
}

void ChartView::set_region(const Region& region) {
  if(m_region == region) {
    return;
  }
  m_region = region;
  if(m_is_auto_scaled) {
    update_auto_scale();
  }
  update_origins();
  update_candlesticks();
  update();
}

bool ChartView::is_auto_scale_enabled() const {
  return m_is_auto_scaled;
}

void ChartView::set_auto_scale(bool auto_scale) {
  m_is_auto_scaled = auto_scale;
  if(m_is_auto_scaled) {
    update_auto_scale();
    update_origins();
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

void ChartView::shift(int dx, int dy) {
  if(!m_region) {
    return;
  }
  auto region = *m_region;
  auto& bottom_right = region.m_bottom_right;
  auto& top_left = region.m_top_left;
  auto dx_points = map_to(dx, 0, get_bottom_right_pixel().x(),
    Scalar(), bottom_right.m_x - top_left.m_x);
  auto dy_points = map_to(dy, 0, get_bottom_right_pixel().y(),
    Scalar(), top_left.m_y - bottom_right.m_y);
  bottom_right.m_x += dx_points;
  bottom_right.m_y -= dy_points;
  top_left.m_x += dx_points;
  top_left.m_y -= dy_points;
  set_region(region);
}

void ChartView::zoom(double factor) {
  if(!m_region) {
    return;
  }
  auto region = *m_region;
  auto& bottom_right = region.m_bottom_right;
  auto& top_left = region.m_top_left;
  auto old_width = bottom_right.m_x - top_left.m_x;
  auto new_width = factor * old_width;
  auto width_change = (new_width - old_width) / 2;
  auto old_height = top_left.m_y - bottom_right.m_y;
  auto new_height = factor * old_height;
  auto height_change = (new_height - old_height) / 2;
  top_left.m_x -= width_change;
  bottom_right.m_x += width_change;
  top_left.m_y += height_change;
  bottom_right.m_y -= height_change;
  set_region(region);
}

void ChartView::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.setFont(m_label_font);
  painter.setPen(Qt::white);
  painter.drawLine(get_bottom_right_pixel().x(), 0,
    get_bottom_right_pixel().x(), get_bottom_right_pixel().y());
  painter.drawLine(0, get_bottom_right_pixel().y(),
    get_bottom_right_pixel().x(), get_bottom_right_pixel().y());
  auto gaps = get_gaps();
  auto x_step = calculate_step(m_model->get_x_axis_type(),
    m_region->m_bottom_right.m_x - m_region->m_top_left.m_x);
  for(auto x = m_region->m_top_left.m_x - m_region->m_top_left.m_x % x_step +
      x_step; x < m_region->m_bottom_right.m_x; x += x_step) {
    auto x_pos = to_pixel(ChartPoint{ x, Scalar() })->x();
    auto time_opt = get_time_by_location(x);
    if(intersects_gap(x_pos, gaps) || !time_opt) {
      continue;
    }
    auto time = *time_opt;
    painter.setPen("#3A3348");
    painter.drawLine(x_pos, 0, x_pos, get_bottom_right_pixel().y());
    painter.setPen(Qt::white);
    painter.drawLine(x_pos, get_bottom_right_pixel().y(), x_pos,
      get_bottom_right_pixel().y() + scale_height(2));
    auto text_width = m_font_metrics.horizontalAdvance(
      m_item_delegate->displayText(to_variant(m_model->get_x_axis_type(), time),
        QLocale()));
    painter.drawText(x_pos - text_width / 2,
      get_bottom_right_pixel().y() + m_font_metrics.height() + scale_height(2),
      m_item_delegate->displayText(to_variant(m_model->get_x_axis_type(), time),
        QLocale()));
  }
  auto y_step = calculate_step(m_model->get_y_axis_type(),
    m_region->m_top_left.m_y - m_region->m_bottom_right.m_y);
  for(auto y = m_region->m_bottom_right.m_y - m_region->m_bottom_right.m_y %
      y_step + y_step; y < m_region->m_top_left.m_y; y += y_step) {
    auto y_pos = to_pixel(ChartPoint{ Scalar(0), y })->y();
    painter.setPen("#3A3348");
    painter.drawLine(0, y_pos, get_bottom_right_pixel().x(), y_pos);
    painter.setPen(Qt::white);
    painter.drawLine(get_bottom_right_pixel().x(), y_pos,
      get_bottom_right_pixel().x() + scale_width(2), y_pos);
    painter.drawText(get_bottom_right_pixel().x() + scale_width(3),
      y_pos + (m_font_metrics.height() / 3), m_item_delegate->displayText(
        to_variant(m_model->get_y_axis_type(), y), QLocale()));
  }
  for(auto& candlestick : m_visible_candlesticks) {
    auto layout = get_candlestick_layout(candlestick);
    if(!layout) {
      continue;
    }
    auto& [open, close, high, low] = *layout;
    if(high.x() < get_bottom_right_pixel().x() && high.y() <
        get_bottom_right_pixel().y()) {
      painter.fillRect(QRect(QPoint(high.x(), high.y()),
        QPoint(high.x(), std::min(low.y(), get_bottom_right_pixel().y() - 1))),
        QColor("#A0A0A0"));
    }
    if(open.y() > close.y() && close.y() < get_bottom_right_pixel().y()) {
      painter.fillRect(QRect(QPoint(open.x(), close.y()),
        QPoint(std::min(close.x() - 1, get_bottom_right_pixel().x() - 1),
          std::min(open.y(), get_bottom_right_pixel().y() - 1))),
        QColor("#8AF5C0"));
      painter.fillRect(QRect(QPoint(open.x() + 1, close.y() + 1),
        QPoint(std::min(close.x() - 2, get_bottom_right_pixel().x() - 1),
          std::min(open.y() - 1, get_bottom_right_pixel().y() - 1))),
        QColor("#1FD37A"));
    } else if(open.y() < get_bottom_right_pixel().y()) {
      painter.fillRect(QRect({open.x(), open.y()},
        QPoint(std::min(close.x() - 1, get_bottom_right_pixel().x() - 1),
          std::min(close.y(), get_bottom_right_pixel().y() - 1))),
        QColor("#FFA7A0"));
      painter.fillRect(QRect(QPoint(open.x() + 1, open.y() + 1),
        QPoint(std::min(close.x() - 2, get_bottom_right_pixel().x() - 1),
          std::min(close.y() - 1, get_bottom_right_pixel().y() - 1))),
        QColor("#EF5357"));
    }
  }
  for(auto& gap : gaps) {
    auto start = std::max(0, to_pixel({ gap.m_start, Scalar(0) })->x());
    auto end = std::min(to_pixel({ gap.m_end, Scalar(0) })->x(),
      get_bottom_right_pixel().x());
    draw_gap(painter, std::max(0, start), std::min(
      get_bottom_right_pixel().x() - 1, end));
  }
  if(m_crosshair_pos && m_crosshair_pos.value().x() <=
      get_bottom_right_pixel().x() && m_crosshair_pos.value().y() <=
      get_bottom_right_pixel().y()) {
    if(m_draw_state == DrawState::OFF ||
        m_draw_state == DrawState::IDLE ||
        m_draw_state == DrawState::NEW) {
      setCursor(CROSSHAIR_CURSOR());
    } else {
      setCursor(HAND_CURSOR());
    }
    painter.setPen(m_dashed_line_pen);
    painter.drawLine(m_crosshair_pos.value().x(), 0,
      m_crosshair_pos.value().x(), get_bottom_right_pixel().y());
    painter.drawLine(0, m_crosshair_pos.value().y(), get_bottom_right_pixel().x(),
      m_crosshair_pos.value().y());
    auto crosshair_chart = to_chart_point(*m_crosshair_pos);
    auto x = crosshair_chart->m_x;
    auto crosshair_time = get_time_by_location(x);
    if(crosshair_time && !intersects_gap(m_crosshair_pos->x(), gaps)) {
      auto x_label = m_item_delegate->displayText(to_variant(
        m_model->get_x_axis_type(), *crosshair_time), QLocale());
      auto x_label_width = m_font_metrics.horizontalAdvance(x_label);
      painter.fillRect(m_crosshair_pos.value().x() - (x_label_width / 2) -
        scale_width(5), get_bottom_right_pixel().y(), x_label_width +
        scale_width(10), scale_height(21), Qt::white);
      painter.fillRect(m_crosshair_pos.value().x(), get_bottom_right_pixel().y(),
        scale_width(1), scale_height(3), Qt::black);
      auto text_width = m_font_metrics.horizontalAdvance(x_label);
      painter.setPen(m_label_text_color);
      painter.drawText(m_crosshair_pos.value().x() - text_width / 2,
        get_bottom_right_pixel().y() + m_font_metrics.height() + scale_height(2),
        x_label);
    } else {
      painter.fillRect(m_crosshair_pos.value().x() - (scale_width(64) / 2),
        get_bottom_right_pixel().y(), scale_width(64), scale_height(21),
        Qt::white);
      painter.fillRect(m_crosshair_pos.value().x(), get_bottom_right_pixel().y(),
        scale_width(1), scale_height(3), Qt::black);
      auto text_width = m_font_metrics.horizontalAdvance(tr("No Activity"));
      painter.setPen(m_label_text_color);
      painter.drawText(m_crosshair_pos.value().x() - text_width / 2,
        get_bottom_right_pixel().y() + m_font_metrics.height() + scale_height(2),
        tr("No Activity"));
    }
    painter.fillRect(get_bottom_right_pixel().x(),
      m_crosshair_pos.value().y() - (scale_height(15) / 2),
      width() - get_bottom_right_pixel().x(), scale_height(15), Qt::white);
    painter.fillRect(get_bottom_right_pixel().x(), m_crosshair_pos.value().y(),
      scale_width(3), scale_height(1), Qt::black);
    auto y = crosshair_chart->m_y;
    auto y_label = m_item_delegate->displayText(to_variant(
      m_model->get_y_axis_type(), y), QLocale());
    painter.setPen(m_label_text_color);
    painter.drawText(get_bottom_right_pixel().x() + scale_width(3),
      m_crosshair_pos.value().y() + (m_font_metrics.height() / 3), y_label);
  } else {
    setCursor(Qt::ArrowCursor);
  }

  painter.setClipRegion({0, 0, get_bottom_right_pixel().x(),
    get_bottom_right_pixel().y()});
  for(auto& line : m_trend_line_model.get_lines()) {
    auto first = to_pixel(std::get<0>(line.m_points));
    auto second = to_pixel(std::get<1>(line.m_points));
    if(!first || !second) {
      continue;
    }
    draw_trend_line(painter, line.m_style, line.m_color,
      first->x(), first->y(), second->x(), second->y());
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
  if(!m_region) {
    m_data_update_promise = m_data_update_promise.then([&](auto result) {
      result.Get();
      return load_first_candlestick().then([&](auto result) {
        auto loaded = result.Get();
        if(loaded) {
          auto& last_candlestick = m_visible_candlesticks.back();
          auto last_location = last_candlestick.get_location();
          auto last_high = last_candlestick.GetHigh();
          auto last_low = last_candlestick.GetLow();
          set_region({ { last_location - Scalar(500), last_high },
            { last_location + Scalar(500), last_low } });
        } else {
          set_region({ { Scalar(0), Scalar(1000) },
            { Scalar(size().width()), Scalar(0) } });
        }
        return std::nullopt;
      });
    });
  }
  QWidget::showEvent(event);
}

Scalar ChartView::get_gap_size() const {
  return Scalar(200);
}

std::optional<ChartView::Gap> ChartView::get_gap_between(
    const PeggedCandlestick& lhs, const PeggedCandlestick& rhs) const {
  if(rhs.get_location() - lhs.get_location() == get_gap_size()) {
    auto start = lhs.get_location() + (lhs.GetEnd() - lhs.GetStart()) /
      (2 * m_time_per_point);
    auto end = rhs.get_location() - (rhs.GetEnd() - rhs.GetStart()) /
      (2 * m_time_per_point);
    return Gap{ start, end };
  }
  return std::nullopt;
}

std::vector<ChartView::Gap> ChartView::get_gaps() const {
  auto result = std::vector<Gap>();
  if(m_visible_candlesticks.empty()) {
    return result;
  }
  if(m_left_candlestick) {
    auto gap = get_gap_between(*m_left_candlestick,
      m_visible_candlesticks.front());
    if(gap) {
      result.push_back(*gap);
    }
  }
  for(auto i = std::size_t(0); i < m_visible_candlesticks.size() - 1; ++i) {
    auto gap = get_gap_between(m_visible_candlesticks[i],
      m_visible_candlesticks[i + 1]);
    if(gap) {
      result.push_back(*gap);
    }
  }
  if(m_right_candlestick) {
    auto gap = get_gap_between(m_visible_candlesticks.back(),
      *m_right_candlestick);
    if(gap) {
      result.push_back(*gap);
    }
  }
  return result;
}

void ChartView::draw_gap(QPainter& painter, int start, int end) {
  painter.fillRect(start, get_bottom_right_pixel().y(), end - start,
    scale_height(3), QColor("#25212E"));
  painter.save();
  painter.setPen(Qt::white);
  painter.drawLine(start, get_bottom_right_pixel().y(), start,
    get_bottom_right_pixel().y() +  scale_height(2));
  if(end <= get_bottom_right_pixel().x()) {
    painter.drawLine(end, get_bottom_right_pixel().y(), end,
      get_bottom_right_pixel().y() + scale_height(2));
  }
  end = min(end, get_bottom_right_pixel().x());
  painter.setPen("#8C8C8C");
  auto slash_count = (static_cast<double>(end) - static_cast<double>(start)) /
    (static_cast<double>(scale_width(4)) +
      static_cast<double>(scale_width(1))) - 1.0;
  auto padding = std::fmod(slash_count, scale_width(4) + scale_width(1)) / 2;
  auto x = start + static_cast<int>(padding) + scale_width(1);
  for(auto i = 0; i < slash_count; ++i) {
    painter.drawImage(x, get_bottom_right_pixel().y(), GAP_SLASH_IMAGE());
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
  if(!first || !second) {
    return;
  }
  auto current_point = to_pixel(m_current_trend_line_point);
  auto first_color = QColor("#25212E");
  auto second_color = QColor("#25212E");
  if(current_point == first) {
    first_color = QColor("#B9B4EC");
  } else if(current_point == second) {
    second_color = QColor("#B9B4EC");
  }
  draw_point(painter, first_color, *first);
  draw_point(painter, second_color, *second);
}

bool ChartView::intersects_gap(int x, const std::optional<std::vector<Gap>>&
    gaps) const {
  auto x_location = to_chart_point({ x, 0 })->m_x;
  auto gap_list = gaps.value_or(get_gaps());
  auto it = std::upper_bound(gap_list.begin(), gap_list.end(), x_location,
    [](auto x, auto& gap) {
      return x < gap.m_start;
    });
  auto result = !gap_list.empty() && it != gap_list.begin() &&
    x_location < (it - 1)->m_end;
  return result;
}

int ChartView::update_intersection(const QPoint& mouse_pos) {
  auto id = m_trend_line_model.find_closest(
    *to_chart_point(*m_crosshair_pos));
  if(id == -1) {
    return id;
  }
  auto line = m_trend_line_model.get(id);
  auto point1 = *to_pixel(std::get<0>(line.m_points));
  auto point2 = *to_pixel(std::get<1>(line.m_points));
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

void ChartView::update_auto_scale() {
  if(!m_region || m_visible_candlesticks.empty()) {
    return;
  }
  auto top = std::max_element(m_visible_candlesticks.begin(),
    m_visible_candlesticks.end(), [](auto& lhs, auto& rhs) {
      return lhs.GetHigh() < rhs.GetHigh();
    })->GetHigh();
  auto bottom = std::min_element(m_visible_candlesticks.begin(),
    m_visible_candlesticks.end(), [](auto& lhs, auto& rhs) {
      return lhs.GetLow() < rhs.GetLow();
    })->GetLow();
  auto region = *m_region;
  region.m_top_left.m_y = top;
  region.m_bottom_right.m_y = bottom;
  if(region != *m_region) {
    set_region(region);
  }
}

void ChartView::update_origins() {
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
    if(m_current_trend_line_point.m_x != Scalar() &&
      m_current_trend_line_point.m_y != Scalar()) {
      m_trend_line_model.set_selected(m_current_trend_line_id);
      m_draw_state = DrawState::POINT;
    } else {
      m_trend_line_model.set_selected(m_current_trend_line_id);
      m_draw_state = DrawState::LINE;
    }
  } else if(m_draw_state == DrawState::IDLE) {
    if(m_trend_line_model.get_selected().empty()) {
      m_current_trend_line_point = *to_chart_point(pos);
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
    m_current_trend_line_point = *to_chart_point(pos);
    m_trend_line_model.update(TrendLine{{m_current_trend_line_point,
      m_current_stationary_point}, line.m_color, line.m_style},
      m_current_trend_line_id);
    m_draw_state = DrawState::IDLE;
  }
}

void ChartView::on_left_mouse_button_release() {
  if(m_draw_state == DrawState::LINE || m_draw_state == DrawState::POINT) {
    m_line_mouse_offset = std::nullopt;
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

void ChartView::update_candlesticks() {
  m_data_update_promise = m_data_update_promise.then([&](auto& result) {
    result.Get();
    drop_left_candlesticks();
    drop_right_candlesticks();
    auto loader = [&](auto& result) {
      result.Get();
      load_left_candlesticks();
      load_right_candlesticks();
      return std::nullopt;
    };
    if(is_empty()) {
      return load_first_candlestick().then(loader);
    } else {
      return QtPromise([] { return std::nullopt; }).then(loader);
    }
  });
}

void ChartView::drop_left_candlesticks() {
  while(!m_visible_candlesticks.empty() &&
      !is_visible(m_visible_candlesticks.front())) {
    auto candlestick = std::move(m_visible_candlesticks.front());
    m_visible_candlesticks.pop_front();
    m_left_candlestick = std::move(candlestick);
  }
}

void ChartView::drop_right_candlesticks() {
  while(!m_visible_candlesticks.empty() &&
      !is_visible(m_visible_candlesticks.back())) {
    auto candlestick = std::move(m_visible_candlesticks.back());
    m_visible_candlesticks.pop_back();
    m_right_candlestick = std::move(candlestick);
  }
}

QtPromise<bool> ChartView::load_first_candlestick() {
  return m_model->load(Scalar(std::numeric_limits<Quantity>::lowest()),
    Scalar(std::numeric_limits<Quantity>::max()),
    SnapshotLimit::FromTail(1)).then([&](auto& result) {
      auto& candlesticks = result.Get();
      if(candlesticks.empty()) {
        return false;
      } else {
        auto& candlestick = candlesticks[0];
        auto info = PeggedCandlestick(std::move(candlestick),
          Scalar(get_bottom_right_pixel().x() / 2));
        m_visible_candlesticks.push_back(std::move(info));
        return true;
      }
    });
}

void ChartView::load_left_candlesticks() {
  if(!m_left_candlestick || is_visible(*m_left_candlestick)) {
    m_data_update_promise = m_data_update_promise.then([&](auto& result) {
      result.Get();
      return m_model->load(Scalar(std::numeric_limits<Quantity>::lowest()),
        get_candlestick_time(**get_leftmost_candlestick()),
        SnapshotLimit::FromTail(2)).then([&](auto& result) {
          auto& candlesticks = result.Get();
          if(m_left_candlestick) {
            auto candlestick = std::move(*m_left_candlestick);
            m_left_candlestick.reset();
            m_visible_candlesticks.push_front(std::move(candlestick));
          }
          if(candlesticks.size() == 2) {
            auto& candlestick = candlesticks[0];
            insert_left_candlestick(std::move(candlestick));
            load_left_candlesticks();
          }
          return std::nullopt;
        });
      });
  }
}

void ChartView::load_right_candlesticks() {
  if(!m_right_candlestick || is_visible(*m_right_candlestick)) {
    m_data_update_promise = m_data_update_promise.then([&](auto& result) {
      result.Get();
      return m_model->load(get_candlestick_time(**get_rightmost_candlestick()),
        Scalar(std::numeric_limits<Quantity>::max()),
        SnapshotLimit::FromHead(2)).then([&](auto& result) {
          auto& candlesticks = result.Get();
          if(m_right_candlestick) {
            auto candlestick = std::move(*m_right_candlestick);
            m_right_candlestick.reset();
            m_visible_candlesticks.push_back(std::move(candlestick));
          }
          if(candlesticks.size() == 2) {
            auto& candlestick = candlesticks[1];
            insert_right_candlestick(std::move(candlestick));
            load_right_candlesticks();
          }
          return std::nullopt;
        });
      });
  }
}

void ChartView::insert_left_candlestick(Candlestick candlestick) {
  auto& first_visible_candlestick = m_visible_candlesticks.front();
  auto distance = (get_candlestick_time(first_visible_candlestick) -
    get_candlestick_time(candlestick)) / m_time_per_point;
  if(distance > get_gap_size()) {
    distance = get_gap_size();
  }
  auto location = first_visible_candlestick.get_location() - distance;
  auto pegged_candlestick = PeggedCandlestick(std::move(candlestick),
    location);
  m_left_candlestick = std::move(pegged_candlestick);
}

void ChartView::insert_right_candlestick(Candlestick candlestick) {
  auto& last_visible_candlestick = m_visible_candlesticks.back();
  auto distance = (get_candlestick_time(candlestick) -
    get_candlestick_time(last_visible_candlestick)) / m_time_per_point;
  if(distance > get_gap_size()) {
    distance = get_gap_size();
  }
  auto location = last_visible_candlestick.get_location() + distance;
  auto pegged_candlestick = PeggedCandlestick(std::move(candlestick),
    location);
  m_right_candlestick = std::move(pegged_candlestick);
}

ChartView::PeggedCandlestick::PeggedCandlestick(Candlestick candlestick,
    Scalar location)
  : Candlestick(std::move(candlestick)),
    m_location(location) {}

void ChartView::PeggedCandlestick::set_location(Scalar location) {
  m_location = location;
}

Scalar ChartView::PeggedCandlestick::get_location() const {
  return m_location;
}

bool ChartView::is_visible(const PeggedCandlestick& candlestick) const {
  auto layout = get_candlestick_layout(candlestick);
  return layout && layout->open.x() < get_bottom_right_pixel().x() &&
    layout->close.x() >= 0;
}

std::optional<QPoint> ChartView::to_pixel(const ChartPoint& point) const {
  if(!m_region) {
    return std::nullopt;
  }
  auto& top_left = m_region->m_top_left;
  auto& bottom_right = m_region->m_bottom_right;
  auto max_x = get_bottom_right_pixel().x();
  auto max_y = get_bottom_right_pixel().y();
  auto x = map_to(point.m_x, top_left.m_x, bottom_right.m_x, 0, max_x);
  auto y = map_to(point.m_y, bottom_right.m_y, top_left.m_y, max_y, 0);
  auto pixel = QPoint(x, y);
  return pixel;
}

std::optional<ChartPoint> ChartView::to_chart_point(
    const QPoint& point) const {
  if(!m_region) {
    return std::nullopt;
  }
  auto& top_left = m_region->m_top_left;
  auto& bottom_right = m_region->m_bottom_right;
  auto max_x = get_bottom_right_pixel().x();
  auto max_y = get_bottom_right_pixel().y();
  auto x = map_to(point.x(), 0, max_x, top_left.m_x, bottom_right.m_x);
  auto y = map_to(point.y(), max_y, 0, bottom_right.m_y, top_left.m_y);
  auto result = ChartPoint{ x, y };
  return result;
}

QPoint ChartView::get_bottom_right_pixel() const {
  auto x = width() + scale_width(4);
  if(m_region) {
    auto& top_left = m_region->m_top_left;
    auto& bottom_right = m_region->m_bottom_right;
    auto y_step = calculate_step(m_model->get_y_axis_type(),
      top_left.m_y - bottom_right.m_y);
    auto y_value = bottom_right.m_y - (bottom_right.m_y % y_step) + y_step;
    x -= m_font_metrics.horizontalAdvance("M") * (
      m_item_delegate->displayText(to_variant(m_model->get_y_axis_type(),
        y_value), QLocale()).length());
  }
  auto y = height() - m_font_metrics.height() - scale_height(9);
  return QPoint(x, y);
}

std::optional<ChartView::CandlestickLayout> ChartView::get_candlestick_layout(
    const PeggedCandlestick& candlestick) const {
  auto location = candlestick.get_location();
  auto span = (candlestick.GetEnd() - candlestick.GetStart()) /
    m_time_per_point;
  auto open = to_pixel({ location - span / 2.,
    candlestick.GetOpen() });
  auto close = to_pixel({ location + span / 2.,
    candlestick.GetClose() });
  auto high = to_pixel({ location, candlestick.GetHigh() });
  auto low = to_pixel({ location, candlestick.GetLow() });
  if(open && close && high && low) {
    return CandlestickLayout{ *open, *close, *high, *low };
  } else {
    return std::nullopt;
  }
}

std::optional<Scalar> ChartView::get_time_by_location(Scalar location) const {
  auto leftmost_candlestick = get_leftmost_candlestick();
  auto rightmost_candlestick = get_rightmost_candlestick();
  if(!leftmost_candlestick || !rightmost_candlestick) {
    return std::nullopt;
  }
  auto lhs = *leftmost_candlestick;
  auto rhs = *rightmost_candlestick;
  if(location < lhs->get_location()) {
    auto lhs_time = get_candlestick_time(*lhs);
    auto time = lhs_time - m_time_per_point * (lhs->get_location() - location);
    return time;
  }
  if(rhs->get_location() < location) {
    auto rhs_time = get_candlestick_time(*rhs);
    auto time = rhs_time + m_time_per_point * (location - rhs->get_location());
    return time;
  }
  if(!m_visible_candlesticks.empty()) {
    auto it = std::upper_bound(m_visible_candlesticks.begin(),
      m_visible_candlesticks.end(), location, [&](auto location,
        auto& candlestick) {
      return location < candlestick.get_location();
    });
    if(it == m_visible_candlesticks.end()) {
      lhs = &m_visible_candlesticks.back();
    } else {
      rhs = &(*it);
      if(it != m_visible_candlesticks.begin()) {
        lhs = &(*(it - 1));
      }
    }
  }
  auto lhs_time = get_candlestick_time(*lhs);
  auto rhs_time = get_candlestick_time(*rhs);
  auto time = map_to(location, lhs->get_location(),
    rhs->get_location(), lhs_time, rhs_time);
  return time;
}

Scalar ChartView::get_candlestick_time(const Candlestick&
    candlestick) {
  return (candlestick.GetStart() + candlestick.GetEnd()) / 2.;
}

std::optional<ChartView::PeggedCandlestick*>
    ChartView::get_leftmost_candlestick() {
  if(m_left_candlestick) {
    return &(*m_left_candlestick);
  } else if(!m_visible_candlesticks.empty()) {
    return &m_visible_candlesticks.front();
  } else {
    return std::nullopt;
  }
}

std::optional<const ChartView::PeggedCandlestick*>
    ChartView::get_leftmost_candlestick() const {
  if(m_left_candlestick) {
    return &(*m_left_candlestick);
  } else if(!m_visible_candlesticks.empty()) {
    return &m_visible_candlesticks.front();
  } else {
    return std::nullopt;
  }
}

std::optional<ChartView::PeggedCandlestick*>
    ChartView::get_rightmost_candlestick() {
  if(m_right_candlestick) {
    return &(*m_right_candlestick);
  } else if(!m_visible_candlesticks.empty()) {
    return &m_visible_candlesticks.back();
  } else if(m_left_candlestick) {
    return &(*m_left_candlestick);
  } else {
    return std::nullopt;
  }
}

std::optional<const ChartView::PeggedCandlestick*>
    ChartView::get_rightmost_candlestick() const {
  if(m_right_candlestick) {
    return &(*m_right_candlestick);
  } else if(!m_visible_candlesticks.empty()) {
    return &m_visible_candlesticks.back();
  } else if(m_left_candlestick) {
    return &(*m_left_candlestick);
  } else {
    return std::nullopt;
  }
}

bool ChartView::is_empty() const {
  return m_visible_candlesticks.empty() && !m_left_candlestick &&
    !m_right_candlestick;
}
