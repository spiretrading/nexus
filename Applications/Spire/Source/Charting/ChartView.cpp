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
    if(type == Scalar::Type::MONEY) {
      return Scalar(Money::ONE);
    } else if(type == Scalar::Type::TIMESTAMP) {
      return Scalar(minutes(10));
    }
    return Scalar();
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
    // TODO: line model
  }
  update();
}

void ChartView::reset_crosshair() {
  m_crosshair_pos = std::nullopt;
}

const std::optional<ChartView::Region>& ChartView::get_region() const {
  return m_region;
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
  auto before = [](auto& lhs, auto& rhs) {
    if(lhs.get_location() >= rhs.get_location()) {
      auto capture = 1;
    }
    if(get_candlestick_time(lhs) >= get_candlestick_time(rhs)) {
      auto capture = 1;
    }
    assert(lhs.get_location() < rhs.get_location());
    assert(lhs.GetEnd() <= rhs.GetStart());
  };
  if(!m_visible_candlesticks.empty()) {
    auto& front = m_visible_candlesticks.front();
    auto& back = m_visible_candlesticks.back();
    if(m_left_candlestick) {
      before(*m_left_candlestick, front);
    }
    if(m_right_candlestick) {
      before(back, *m_right_candlestick);
    }
    for(auto i = std::size_t(0); i < m_visible_candlesticks.size() - 1; ++i) {
      auto& lhs = m_visible_candlesticks[i];
      auto& rhs = m_visible_candlesticks[i + 1];
      before(lhs, rhs);
    }
  }
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
  painter.setPen(Qt::white);

  painter.drawLine(get_top_right_pixel().x(), 0, get_top_right_pixel().x(),
    get_top_right_pixel().y());
  painter.drawLine(0, get_top_right_pixel().y(), get_top_right_pixel().x(),
    get_top_right_pixel().y());

  painter.setPen(Qt::white);
  for(auto& candlestick : m_visible_candlesticks) {
    auto layout = get_candlestick_layout(candlestick);
    if(!layout) {
      continue;
    }
    auto& [open, close, high, low] = *layout;
    auto color = QColor([&] {
      if(open.y() > close.y()) {
        return "#EF5357";
      } else {
        return "#1FD37A";
      }
    }());
    painter.fillRect(QRect(open, close), color);
  }
  if(m_crosshair_pos && m_crosshair_pos.value().x() <=
      get_top_right_pixel().x() && m_crosshair_pos.value().y() <=
      get_top_right_pixel().y()) {
    if(m_draw_state == DrawState::OFF ||
        m_draw_state == DrawState::IDLE ||
        m_draw_state == DrawState::NEW) {
      setCursor(CROSSHAIR_CURSOR());
    } else {
      setCursor(HAND_CURSOR());
    }
    painter.setPen(m_dashed_line_pen);
    painter.drawLine(m_crosshair_pos.value().x(), 0,
      m_crosshair_pos.value().x(), get_top_right_pixel().y());
    painter.drawLine(0, m_crosshair_pos.value().y(), get_top_right_pixel().x(),
      m_crosshair_pos.value().y());
    auto x = map_to(m_crosshair_pos->x(), 0, get_top_right_pixel().x(),
      m_region->m_top_left.m_x, m_region->m_bottom_right.m_x);
    auto crosshair_time = get_time_by_location(x);
    if(crosshair_time && !intersects_gap(m_crosshair_pos->x())) {
      auto x_label = m_item_delegate->displayText(to_variant(
        m_model->get_x_axis_type(), *crosshair_time), QLocale());
      auto x_label_width = m_font_metrics.horizontalAdvance(x_label);
      painter.fillRect(m_crosshair_pos.value().x() - (x_label_width / 2) -
        scale_width(5), get_top_right_pixel().y(), x_label_width +
        scale_width(10), scale_height(21), Qt::white);
      painter.fillRect(m_crosshair_pos.value().x(), get_top_right_pixel().y(),
        scale_width(1), scale_height(3), Qt::black);
      auto text_width = m_font_metrics.horizontalAdvance(x_label);
      painter.setPen(m_label_text_color);
      painter.drawText(m_crosshair_pos.value().x() - text_width / 2,
        get_top_right_pixel().y() + m_font_metrics.height() + scale_height(2),
        x_label);
    } else {
      painter.fillRect(m_crosshair_pos.value().x() - (scale_width(64) / 2),
        get_top_right_pixel().y(), scale_width(64), scale_height(21),
        Qt::white);
      painter.fillRect(m_crosshair_pos.value().x(), get_top_right_pixel().y(),
        scale_width(1), scale_height(3), Qt::black);
      auto text_width = m_font_metrics.horizontalAdvance(tr("No Activity"));
      painter.setPen(m_label_text_color);
      painter.drawText(m_crosshair_pos.value().x() - text_width / 2,
        get_top_right_pixel().y() + m_font_metrics.height() + scale_height(2),
        tr("No Activity"));
    }
    painter.fillRect(get_top_right_pixel().x(),
      m_crosshair_pos.value().y() - (scale_height(15) / 2),
      width() - get_top_right_pixel().x(), scale_height(15), Qt::white);
    painter.fillRect(get_top_right_pixel().x(), m_crosshair_pos.value().y(),
      scale_width(3), scale_height(1), Qt::black);
    auto y_label = m_item_delegate->displayText(to_variant(
      m_model->get_y_axis_type(), Scalar(0)), QLocale());
    painter.setPen(m_label_text_color);
    painter.drawText(get_top_right_pixel().x() + scale_width(3),
      m_crosshair_pos.value().y() + (m_font_metrics.height() / 3), y_label);
  } else {
    setCursor(Qt::ArrowCursor);
  }

  painter.setClipRegion({0, 0, get_top_right_pixel().x(),
    get_top_right_pixel().y()});

  // TODO: draw lines
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

bool ChartView::intersects_gap(int x) const {
  return false;
}

void ChartView::update_auto_scale() {
  if(!m_region || m_visible_candlesticks.empty()) {
    return;
  }
  auto auto_scale_top = m_visible_candlesticks.front().GetHigh();
  auto auto_scale_bottom = m_visible_candlesticks.front().GetLow();
  for(auto& candle : m_visible_candlesticks) {
    auto_scale_top = max(auto_scale_top, candle.GetHigh());
    auto_scale_bottom = min(auto_scale_bottom, candle.GetLow());
  }
  m_region->m_top_left.m_y = auto_scale_top;
  m_region->m_bottom_right.m_y = auto_scale_bottom;
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
    auto f = std::ofstream("log.txt", std::ios::app);
    f << "promise execution" << std::endl;
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
      !is_visible(m_visible_candlesticks.front().get_location())) {
    auto candlestick = std::move(m_visible_candlesticks.front());
    m_visible_candlesticks.pop_front();
    m_left_candlestick = std::move(candlestick);
  }
}

void ChartView::drop_right_candlesticks() {
  while(!m_visible_candlesticks.empty() &&
      !is_visible(m_visible_candlesticks.back().get_location())) {
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
          Scalar(get_top_right_pixel().x() / 2));
        m_visible_candlesticks.push_back(std::move(info));
        return true;
      }
    });
}

void ChartView::load_left_candlesticks() {
  if(!m_left_candlestick || is_visible(m_left_candlestick->get_location())) {
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
  if(!m_right_candlestick || is_visible(m_right_candlestick->get_location())) {
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
  if(distance > Scalar(200)) {
    distance = Scalar(200);
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
  if(distance > Scalar(200)) {
    distance = Scalar(200);
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

bool ChartView::is_visible(Scalar location) const {
  return location <= m_region->m_bottom_right.m_x &&
    location >= m_region->m_top_left.m_x;
}

std::optional<QPoint> ChartView::to_pixel(const ChartPoint& point) const {
  if(!m_region) {
    return std::nullopt;
  }
  auto& top_left = m_region->m_top_left;
  auto& bottom_right = m_region->m_bottom_right;
  auto max_x = get_top_right_pixel().x();
  auto max_y = get_top_right_pixel().y();
  auto x = map_to(point.m_x, top_left.m_x, bottom_right.m_x, 0, max_x);
  auto y = map_to(point.m_y, bottom_right.m_y, top_left.m_y, 0, max_y);
  auto pixel = QPoint(x, y);
  return pixel;
}

QPoint ChartView::get_top_right_pixel() const {
  return QPoint(size().width() - 30, size().height() - 30);
}

std::optional<ChartView::CandlestickLayout> ChartView::get_candlestick_layout(
    const PeggedCandlestick& candlestick) const {
  auto location = candlestick.get_location();
  auto span = (candlestick.GetStart() - candlestick.GetEnd()) /
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
