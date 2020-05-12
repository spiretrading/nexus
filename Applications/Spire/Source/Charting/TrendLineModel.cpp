#include "Spire/Charting/TrendLineModel.hpp"
#include <algorithm>

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  double cast_value(Scalar value) {
    return static_cast<double>(
      static_cast<Quantity>(static_cast<Scalar>(value)));
  }

  QPointF cast_point(const ChartPoint& point) {
    return {cast_value(point.m_x), cast_value(point.m_y)};
  }
}

TrendLineModel::TrendLineModel()
  : m_last_id(0) {}

int TrendLineModel::add(const TrendLine& line) {
  m_trend_lines.push_back({line, ++m_last_id, State::UNSELECTED});
  m_update_signal();
  return m_last_id;
}

TrendLine TrendLineModel::get(int id) {
  auto iter = find_id(id);
  if(iter == m_trend_lines.end()) {
    throw std::invalid_argument("TrendLine not found.");
  }
  return iter->m_trend_line;
}

std::vector<TrendLine> TrendLineModel::get_lines() const {
  auto lines = std::vector<TrendLine>();
  for(auto& line : m_trend_lines) {
    lines.push_back(line.m_trend_line);
  }
  return lines;
}

std::vector<int> TrendLineModel::get_selected() const {
  auto selected = std::vector<int>();
  for(auto& line : m_trend_lines) {
    if(line.m_state == State::SELECTED) {
      selected.push_back(line.m_id);
    }
  }
  return selected;
}

int TrendLineModel::find_closest(const ChartPoint& point) const {
  if(m_trend_lines.empty()) {
    return -1;
  }
  auto closest_id = m_trend_lines.front().m_id;
  auto closest_distance = std::numeric_limits<double>::infinity();
  auto qpoint = cast_point(point);
  for(auto& line : m_trend_lines) {
    auto dist_squared = std::numeric_limits<double>::infinity();
    auto point_distance_squared = std::numeric_limits<double>::infinity();
    auto line_slope = slope(
      cast_point(std::get<0>(line.m_trend_line.m_points)),
      cast_point(std::get<1>(line.m_trend_line.m_points)));
    auto line_point1 = cast_point(std::get<0>(line.m_trend_line.m_points));
    auto line_point2 = cast_point(std::get<1>(line.m_trend_line.m_points));
    auto line_b = y_intercept(line_point1, line_slope);
    point_distance_squared = closest_point_distance_squared(qpoint,
      line_point1, line_point2);
    if(std::isinf(line_slope)) {
      if(is_within_interval(qpoint.y(), line_point1.y(), line_point2.y())) {
        dist_squared = distance_squared(qpoint,
          {line_point1.x(), qpoint.y()});
      }
    } else if(line_slope == 0) {
      if(is_within_interval(qpoint.x(), line_point1.x(), line_point2.x())) {
        dist_squared =  distance_squared(qpoint,
          {qpoint.x(), line_point1.y()});
      }
    } else {
      auto line_point_x =
        (qpoint.x() + line_slope * qpoint.y() - line_slope * line_b) /
        (line_slope * line_slope + 1);
      if(is_within_interval(line_point_x, line_point1.x(), line_point2.x())) {
        dist_squared = distance_squared(qpoint, {line_point_x,
          calculate_y(line_slope, line_point_x, line_b)});
      }
    }
    auto min_distance_squared = std::min(dist_squared, point_distance_squared);
    if(min_distance_squared < closest_distance) {
      closest_id = line.m_id;
      closest_distance = min_distance_squared;
    }
  }
  return closest_id;
}

void TrendLineModel::remove(int id) {
  auto iter = find_id(id);
  if(iter != m_trend_lines.end()) {
    m_trend_lines.erase(iter);
  }
  m_update_signal();
}

void TrendLineModel::set_selected(int id) {
  auto iter = find_id(id);
  if(iter != m_trend_lines.end()) {
    iter->m_state = State::SELECTED;
  }
  m_update_signal();
}

void TrendLineModel::unset_selected(int id) {
  auto iter = find_id(id);
  if(iter != m_trend_lines.end()) {
    iter->m_state = State::UNSELECTED;
  }
  m_update_signal();
}

void TrendLineModel::toggle_selection(int id) {
  if(is_selected(id)) {
    unset_selected(id);
  } else {
    set_selected(id);
  }
}

void TrendLineModel::clear_selected() {
  for(auto& line : m_trend_lines) {
    line.m_state = State::UNSELECTED;
  }
}

void TrendLineModel::update(const TrendLine& line, int id) {
  auto iter = find_id(id);
  if(iter != m_trend_lines.end()) {
    iter->m_trend_line = line;
  }
  m_update_signal();
}

connection TrendLineModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

std::vector<TrendLineModel::TrendLineEntry>::iterator
  TrendLineModel::find_id(int id) {
    return std::find_if(m_trend_lines.begin(), m_trend_lines.end(),
      [&] (auto& line) {
        return id == line.m_id;
      });
}

bool TrendLineModel::is_selected(int id) const {
  auto selected = std::find_if(m_trend_lines.begin(), m_trend_lines.end(),
    [&] (auto& line) {
      return id == line.m_id && line.m_state == State::SELECTED;
    });
  return selected != m_trend_lines.end();
}

void TrendLineModel::set_selected_status(int id, State state) {
  auto iter = find_id(id);
  if(iter != m_trend_lines.end()) {
    iter->m_state = state;
  }
}
