#include "Spire/Charting/ChartView.hpp"
#include <algorithm>
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
  auto GAP_WIDTH() {
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

  Scalar calculate_step(Scalar::Type type, Scalar range) {
    if(type == Scalar::Type::MONEY) {
      return Scalar(Money::ONE);
    } else if(type == Scalar::Type::TIMESTAMP) {
      return Scalar(minutes(10));
    }
    return Scalar();
  }

  Scalar get_lowest(Scalar::Type type) {
    if(type == Scalar::Type::DURATION) {
      return Scalar(std::numeric_limits<time_duration>::lowest());
    } else if(type == Scalar::Type::MONEY) {
      return Scalar(std::numeric_limits<Money>::lowest());
    } else if(type == Scalar::Type::QUANTITY) {
      return Scalar(std::numeric_limits<Quantity>::lowest());
    } else if(type == Scalar::Type::TIMESTAMP) {
      return Scalar(std::numeric_limits<ptime>::lowest());
    }
    return Scalar(std::numeric_limits<Quantity>::lowest());
  }

  Scalar get_highest(Scalar::Type type) {
    if(type == Scalar::Type::DURATION) {
      return Scalar(std::numeric_limits<time_duration>::max());
    } else if(type == Scalar::Type::MONEY) {
      return Scalar(std::numeric_limits<Money>::max());
    } else if(type == Scalar::Type::QUANTITY) {
      return Scalar(std::numeric_limits<Quantity>::max());
    } else if(type == Scalar::Type::TIMESTAMP) {
      return Scalar(std::numeric_limits<ptime>::max());
    }
    return Scalar(std::numeric_limits<Quantity>::max());
  }

  Scalar calculate_density(const ChartView::Region& region, const QSize& size) {
    return (region.m_bottom_right.m_x - region.m_top_left.m_x) / size.width();
  }

  void foo(Scalar x) {
    qDebug() << CustomVariantItemDelegate().displayText(
      QVariant::fromValue(static_cast<ptime>(x)));
  }

  void boo(Scalar x) {
    qDebug() << CustomVariantItemDelegate().displayText(
      QVariant::fromValue(static_cast<time_duration>(x)));
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
  return to_chart_point(m_region, m_extended_region,
    QSize(m_bottom_right_pixel.x(), m_bottom_right_pixel.y()), m_gaps, point);
}

QPoint ChartView::to_pixel(const ChartPoint& point) const {
  return to_pixel(m_region,
    QSize(m_bottom_right_pixel.x(), m_bottom_right_pixel.y()), m_gaps, point);
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
          to_pixel(std::get<0>(line.m_points)) - *m_crosshair_pos,
          to_pixel(std::get<1>(line.m_points)) - *m_crosshair_pos};
      } else {
        line.m_points = {
          to_chart_point(*m_crosshair_pos + m_line_mouse_offset->m_first),
          to_chart_point(*m_crosshair_pos + m_line_mouse_offset->m_second)};
        m_trend_line_model.update(line, m_current_trend_line_id);
      }
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
  m_crosshair_pos = std::nullopt;
}

const ChartView::Region& ChartView::get_region() const {
  return m_region;
}

void ChartView::set_region(const Region& region) {
  if(region == m_region) {
    return;
  }
  commit_region(region);
  m_region_updates = m_region_updates.then([=] (auto&& result) {
    m_model->load(get_lowest(m_model->get_x_axis_type()), region.m_top_left.m_x,
      SnapshotLimit::FromTail(1)).then([=] (
          std::vector<Candlestick> left_candlestick) {
        return load_region(region, calculate_density(region,
          QSize(m_bottom_right_pixel.x(), m_bottom_right_pixel.y())),
          std::move(left_candlestick), {});
      });
  });
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

void ChartView::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.setFont(m_label_font);
  painter.setPen(Qt::white);
  painter.drawLine(m_bottom_right_pixel.x(), 0, m_bottom_right_pixel.x(),
    m_bottom_right_pixel.y());
  painter.drawLine(0, m_bottom_right_pixel.y(), m_bottom_right_pixel.x(),
    m_bottom_right_pixel.y());
  if(m_x_range <= Scalar(0) || m_y_range <= Scalar(0)) {
    return;
  }
  for(auto y : m_y_axis_values) {
    auto y_pos = map_to(y, m_region.m_bottom_right.m_y,
      m_region.m_top_left.m_y, m_bottom_right_pixel.y(), 0);
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
    auto x_pos = static_cast<int>(to_pixel({x, Scalar()}).x());
    if(x_pos < m_bottom_right_pixel.x() && !intersects_gap(x_pos)) {
      painter.setPen("#3A3348");
      painter.drawLine(x_pos, 0, x_pos, m_bottom_right_pixel.y());
      painter.setPen(Qt::white);
      painter.drawLine(x_pos, m_bottom_right_pixel.y(), x_pos,
        m_bottom_right_pixel.y() + scale_height(2));
      auto text_width = m_font_metrics.horizontalAdvance(
        m_item_delegate->displayText(to_variant(m_model->get_x_axis_type(), x),
        QLocale()));
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
    if(candlestick.GetEnd() >= m_region.m_top_left.m_x && start_x <=
        m_bottom_right_pixel.x()) {
      auto high = map_to(candlestick.GetHigh(), m_region.m_bottom_right.m_y,
        m_region.m_top_left.m_y, m_bottom_right_pixel.y(), 0);
      auto low = map_to(candlestick.GetLow(), m_region.m_bottom_right.m_y,
        m_region.m_top_left.m_y, m_bottom_right_pixel.y(), 0);
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
    auto start = to_pixel({gap.m_start, Scalar()}).x();
    if(start < m_bottom_right_pixel.x()) {
      draw_gap(painter, start, to_pixel({gap.m_end, Scalar()}).x());
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
      auto x_label_width = m_font_metrics.horizontalAdvance(x_label);
      painter.fillRect(m_crosshair_pos.value().x() - (x_label_width / 2) -
        scale_width(5), m_bottom_right_pixel.y(), x_label_width +
        scale_width(10), scale_height(21), Qt::white);
      painter.fillRect(m_crosshair_pos.value().x(), m_bottom_right_pixel.y(),
        scale_width(1), scale_height(3), Qt::black);
      auto text_width = m_font_metrics.horizontalAdvance(x_label);
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
      auto text_width = m_font_metrics.horizontalAdvance(tr("No Activity"));
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
  if(m_region == Region{}) {
    auto current_time = boost::posix_time::second_clock::local_time();
    auto bottom_right = ChartPoint(Scalar(current_time),
      Scalar(Nexus::Money(0)));
    auto top_left = ChartPoint(
      Scalar(current_time - boost::posix_time::hours(1)),
      Scalar(Nexus::Money(1)));
    set_region({top_left, bottom_right});
  }
  QWidget::showEvent(event);
}

QPoint ChartView::to_pixel(const Region& region, const QSize& size,
    const std::vector<Gap>& gaps, const ChartPoint& point) {
  if(point.m_x < region.m_top_left.m_x) {
    auto adjusted_region = Region{{point.m_x, point.m_y},
      {point.m_x + (region.m_bottom_right.m_x - region.m_top_left.m_x),
       point.m_y + (region.m_bottom_right.m_y - region.m_top_left.m_y)}};
    auto adjusted_point = to_pixel(adjusted_region, size, gaps,
      {region.m_top_left.m_x, region.m_top_left.m_y});
    return {-adjusted_point.x(), -adjusted_point.y()};
  }
  auto x = map_to(point.m_x, region.m_top_left.m_x, region.m_bottom_right.m_x,
    0, size.width() - 1);
  auto y = map_to(point.m_y, region.m_bottom_right.m_y, region.m_top_left.m_y,
    size.height() - 1, 0);
  for(auto& gap : gaps) {
    if(gap.m_start < point.m_x && gap.m_end > point.m_x) {
      x = to_pixel(region, size, gaps, {gap.m_start, point.m_y}).x() +
        static_cast<int>(std::round((GAP_WIDTH() * (point.m_x - gap.m_start)) /
        (gap.m_end - gap.m_start)));
      return {x, y};
    }
    if(gap.m_end >= region.m_top_left.m_x) {
      if(point.m_x > gap.m_start) {
        auto visible_start = std::max(gap.m_start, region.m_top_left.m_x);
        auto gap_start = map_to(visible_start, region.m_top_left.m_x,
          region.m_bottom_right.m_x, 0, size.width() - 1);
        auto gap_end = map_to(gap.m_end, region.m_top_left.m_x,
          region.m_bottom_right.m_x, 0, size.width() - 1);
        x -= gap_end - gap_start - static_cast<int>(std::round(GAP_WIDTH() *
          ((gap.m_end - visible_start) / (gap.m_end - gap.m_start))));
      } else {
        break;
      }
    }
  }
  return {x, y};
}

ChartPoint ChartView::to_chart_point(const Region& region,
    const Region& extended_region, const QSize& size,
    const std::vector<Gap>& gaps, const QPoint& point) {
  auto y = map_to(point.y(), size.height() - 1, 0, region.m_bottom_right.m_y,
    region.m_top_left.m_y);
  auto left_x_pixel = 0;
  auto left_x_chart_value = region.m_top_left.m_x;
  auto x = [&] {
    if(!gaps.empty()) {
      auto gap_start_pixel = to_pixel(region, size, gaps,
        {gaps.front().m_start, y}).x();
      if(point.x() < gap_start_pixel) {
        return extended_region.m_top_left.m_x + point.x() *
          calculate_density(region, size);
      }
    }
    for(auto& gap : gaps) {
      auto gap_start_pixel = to_pixel(region, size, gaps, {gap.m_start, y}).x();
      if(point.x() <= gap_start_pixel) {
        if(point.x() < 0 && gap_start_pixel < 0) {
          qDebug() << "uh oh";
        }
        if(gap_start_pixel == 0 && left_x_pixel == 0) {
          return extended_region.m_top_left.m_x + point.x() *
            calculate_density(region, size);
        } else {
          return map_to(point.x(), left_x_pixel, gap_start_pixel,
            left_x_chart_value, gap.m_start);
        }
      }
      auto gap_end_pixel = to_pixel(region, size, gaps, {gap.m_end, y}).x();
      if(point.x() < gap_end_pixel) {
        return map_to(point.x(), gap_start_pixel, gap_end_pixel, gap.m_start,
          gap.m_end);
      }
      left_x_pixel = gap_end_pixel;
      left_x_chart_value = gap.m_end;
    }
    return map_to(point.x(), left_x_pixel, size.width() - 1, left_x_chart_value,
      extended_region.m_bottom_right.m_x);
  }();
  return {x, y};
}

void ChartView::commit_region(const Region& region) {
  m_region = region;
  if(m_is_auto_scaled) {
    update_auto_scale();
  }
  commit_extended_region(m_region);
  update_origins();
  update();
}

void ChartView::commit_extended_region(const Region& region) {
  m_extended_region.m_top_left = region.m_top_left;
  auto density = calculate_density(region,
    QSize(m_bottom_right_pixel.x(), m_bottom_right_pixel.y()));
  auto base_region = Region{region.m_top_left, {
    region.m_top_left.m_x + m_bottom_right_pixel.x() * density,
    region.m_bottom_right.m_y}};
  auto has_visible_gap = false;
  for(auto i = m_gaps.rbegin(); i != m_gaps.rend(); ++i) {
    auto end_pixel = to_pixel(base_region,
      QSize(m_bottom_right_pixel.x(), m_bottom_right_pixel.y()), m_gaps,
      {i->m_end, base_region.m_top_left.m_y}).x();
    if(end_pixel >= 0 && end_pixel < m_bottom_right_pixel.x()) {
      has_visible_gap = true;
      m_extended_region.m_bottom_right.m_x = i->m_end +
        (m_bottom_right_pixel.x() - end_pixel) * density;
    } else if(end_pixel < 0) {
      break;
    }
  }
  if(!has_visible_gap) {
    m_extended_region.m_bottom_right.m_x =
      m_extended_region.m_top_left.m_x + m_bottom_right_pixel.x() *
      density;
  }
}

QtPromise<void> ChartView::load_region(Region region, Scalar density,
    std::vector<Candlestick> candlesticks, std::vector<Gap> gaps) {
  auto start = [&] {
    if(candlesticks.empty()) {
      return region.m_top_left.m_x;
    } else {
      return candlesticks.back().GetEnd();
    }
  }();
  return m_model->load(start, region.m_bottom_right.m_x,
    SnapshotLimit::Unlimited()).then(
    [=] (std::vector<Candlestick> next_candlesticks) mutable {
      if(!candlesticks.empty() && !next_candlesticks.empty() &&
          candlesticks.back().GetStart() ==
          next_candlesticks.front().GetStart()) {
        next_candlesticks.erase(next_candlesticks.begin());
      }
      if(next_candlesticks.empty()) {
        return m_model->load(region.m_bottom_right.m_x,
          get_highest(m_model->get_x_axis_type()), SnapshotLimit::FromHead(1));
      }
      return QtPromise(std::move(next_candlesticks));
    }).then([=, candlesticks = std::move(candlesticks)] (
        std::vector<Candlestick> next_candlesticks) mutable {
      auto i = candlesticks.size();
      candlesticks.insert(candlesticks.end(),
        std::make_move_iterator(next_candlesticks.begin()),
        std::make_move_iterator(next_candlesticks.end()));
      while(i != candlesticks.size()) {
        auto& candlestick = candlesticks[i];
        if(i != 0 && candlestick.GetStart() != candlesticks[i - 1].GetEnd()) {
          auto gap = Gap{candlesticks[i - 1].GetEnd(),
            candlestick.GetStart()};
          gaps.push_back(gap);
        }
        ++i;
      }
      auto position = [&] {
        if(next_candlesticks.empty()) {
          return m_bottom_right_pixel.x();
        }
        auto base_region = Region{region.m_top_left, {
          region.m_top_left.m_x + m_bottom_right_pixel.x() * density,
          region.m_bottom_right.m_y}};
        return to_pixel(base_region,
          QSize(m_bottom_right_pixel.x(), m_bottom_right_pixel.y()), gaps,
          ChartPoint{candlesticks.back().GetEnd(), Scalar()}).x();
      }();
      auto remaining_position = m_bottom_right_pixel.x() - position;
      if(remaining_position > 0) {
        if(position >= 0) {
          region.m_bottom_right.m_x = candlesticks.back().GetEnd() +
            remaining_position * density;
        }
        return load_region(region, density, std::move(candlesticks),
          std::move(gaps));
      } else {
        m_candlesticks = std::move(candlesticks);
        m_gaps = std::move(gaps);
        if(m_is_auto_scaled) {
          update_auto_scale();
        }
        commit_extended_region(region);
        update_origins();
        update();
        return QtPromise();
      }
    });
}

void ChartView::draw_gap(QPainter& painter, int start, int end) {
  painter.fillRect(start, m_bottom_right_pixel.y(), end - start + 1,
    scale_height(3), QColor("#25212E"));
  painter.save();
  painter.setPen(Qt::white);
  painter.drawLine(start, m_bottom_right_pixel.y(), start,
    m_bottom_right_pixel.y() + scale_height(2));
  if(end <= m_bottom_right_pixel.x()) {
    painter.drawLine(end, m_bottom_right_pixel.y(), end,
      m_bottom_right_pixel.y() + scale_height(2));
  }
  end = std::min(end, m_bottom_right_pixel.x());
  painter.setPen("#8C8C8C");
  auto slash_count = static_cast<double>(end - start) /
    (scale_width(4) + scale_width(1)) - 1.0;
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
  m_region.m_top_left.m_y = auto_scale_top;
  m_region.m_bottom_right.m_y = auto_scale_bottom;
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
  m_x_range = m_extended_region.m_bottom_right.m_x - m_region.m_top_left.m_x;
  m_x_axis_step = calculate_step(m_model->get_x_axis_type(), m_x_range);
  m_y_range = m_region.m_top_left.m_y - m_region.m_bottom_right.m_y;
  m_y_axis_step = calculate_step(m_model->get_y_axis_type(), m_y_range);
  auto x_value = m_region.m_top_left.m_x - (m_region.m_top_left.m_x %
    m_x_axis_step) + m_x_axis_step;
  m_x_axis_values.clear();
  while(x_value <= m_extended_region.m_bottom_right.m_x) {
    for(auto& gap : m_gaps) {
      if(gap.m_start < x_value && x_value < gap.m_end) {
        x_value = gap.m_end;
        break;
      }
    }
    m_x_axis_values.push_back(x_value);
    x_value += m_x_axis_step;
  }
  auto y_value = m_region.m_bottom_right.m_y - (m_region.m_bottom_right.m_y %
    m_y_axis_step) + m_y_axis_step;
  m_y_axis_values.clear();
  auto old_x_origin = m_bottom_right_pixel.x();
  m_bottom_right_pixel.setX(width() - (m_font_metrics.horizontalAdvance("M") *
    (m_item_delegate->displayText(to_variant(m_model->get_y_axis_type(),
    y_value), QLocale()).length()) - scale_width(4)));
  auto top_label = m_region.m_top_left.m_y - (m_region.m_top_left.m_y %
    m_y_axis_step);
  while(y_value <= top_label) {
    m_y_axis_values.push_back(y_value);
    auto origin = width() - (m_font_metrics.horizontalAdvance("M") * (
      m_item_delegate->displayText(to_variant(m_model->get_y_axis_type(),
      y_value), QLocale()).length()) - scale_width(4));
    m_bottom_right_pixel.setX(std::min(m_bottom_right_pixel.x(), origin));
    y_value += m_y_axis_step;
  }
  m_bottom_right_pixel.setY(height() - m_font_metrics.height() -
    scale_height(9));
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
      m_current_trend_line_point = to_chart_point(pos);
      m_current_stationary_point = m_current_trend_line_point;
      m_current_trend_line_id = m_trend_line_model.add(
        TrendLine({m_current_trend_line_point, m_current_trend_line_point},
        m_current_trend_line_color, m_current_trend_line_style));
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

void Spire::translate(ChartView& view, const QPoint& offset) {
  if(offset.x() == 0) {
    return;
  }
  static int I = 0;
  ++I;
  qDebug() << "Translate: " << I;
  qDebug() << "Offset: " << offset;
  auto region = view.get_region();
  qDebug() << "Region:";
  foo(region.m_top_left.m_x);
  foo(region.m_bottom_right.m_x);
  qDebug() << "ToChartPoint";
  foo(view.to_chart_point({-offset.x(), 0}).m_x);
  auto delta = region.m_top_left.m_x -
    view.to_chart_point({-offset.x(), 0}).m_x;
  if(static_cast<time_duration>(delta) >= minutes(30) &&
      static_cast<ptime>(region.m_top_left.m_x - delta).time_of_day().hours()
      == 0) {
    view.to_chart_point({-offset.x(), 0});
  }
  boo(delta);
  region.m_top_left.m_x -= delta;
  region.m_bottom_right.m_x -= delta;
  qDebug() << "New Region:";
  foo(region.m_top_left.m_x);
  foo(region.m_bottom_right.m_x);
  view.set_region(region);
}

void Spire::zoom(ChartView& view, double factor) {}
