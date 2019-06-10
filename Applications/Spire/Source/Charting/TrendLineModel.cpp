#include "Spire/Charting/TrendLineModel.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

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

bool TrendLineModel::is_selected(int id) {
  auto selected = std::find_if(m_trend_lines.begin(), m_trend_lines.end(),
    [&] (auto& line) {
      return id == line.m_id && line.m_state == State::SELECTED;
    });
  if(selected == m_trend_lines.end()) {
    return false;
  }
  return true;
}

int TrendLineModel::find_closest(const ChartPoint& point) const {
  if(m_trend_lines.empty()) {
    return -1;
  }
  auto closest_id = m_trend_lines.front().m_id;
  auto closest_distance = std::numeric_limits<Quantity>::infinity();
  auto point_x = static_cast<Quantity>(point.m_x);
  auto point_y = static_cast<Quantity>(point.m_y);
  for(auto& line : m_trend_lines) {
    auto dist_squared = std::numeric_limits<Quantity>::infinity();
    auto point_distance_squared = std::numeric_limits<Quantity>::infinity();
    auto line_slope = 
      slope(static_cast<Quantity>(std::get<0>(line.m_trend_line.m_points).m_x),
      static_cast<Quantity>(std::get<0>(line.m_trend_line.m_points).m_y),
      static_cast<Quantity>(std::get<1>(line.m_trend_line.m_points).m_x),
      static_cast<Quantity>(std::get<1>(line.m_trend_line.m_points).m_y));
    auto line_x1 = static_cast<Quantity>(
      std::get<0>(line.m_trend_line.m_points).m_x);
    auto line_y1 = static_cast<Quantity>(
      std::get<0>(line.m_trend_line.m_points).m_y);
    auto line_x2 = static_cast<Quantity>(
      std::get<1>(line.m_trend_line.m_points).m_x);
    auto line_y2 = static_cast<Quantity>(
      std::get<1>(line.m_trend_line.m_points).m_y);
    auto line_b = y_intercept(line_x1, line_y1, line_slope);
    point_distance_squared = Abs(closest_point_distance_squared(point_x,
      point_y, line_x1, line_y1, line_x2, line_y2));
    if(std::isinf(static_cast<double>(line_slope))) {
      if(is_within_interval(point_y, line_y1, line_y2)) {
        dist_squared = Abs(
          distance_squared(point_x, point_y, line_x1, point_y));
      }
    } else if(line_slope == Quantity(0)) {
      if(is_within_interval(point_x, line_x1, line_x2)) {
        dist_squared = Abs(
          distance_squared(point_x, point_y, point_x, line_y1));
      }
    } else {
      auto line_point_x =
        (point_x + line_slope * point_y - line_slope * line_b) /
        (line_slope * line_slope + 1);
      if(is_within_interval(line_point_x, line_x1, line_x2)) {
        dist_squared = Abs(distance_squared(point_x, point_y,
          line_point_x, calculate_y(line_slope, line_point_x, line_b)));
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

void TrendLineModel::invert_selection(int id) {
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

void TrendLineModel::set_selected_status(int id, State state) {
  auto iter = find_id(id);
  if(iter != m_trend_lines.end()) {
    iter->m_state = state;
  }
}
