#include "spire/charting/trend_line_model.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  Quantity slope(const TrendLine& line) {
    return Quantity(
      (std::get<1>(line.m_points).m_y - std::get<0>(line.m_points).m_y)
      / (std::get<1>(line.m_points).m_x - std::get<0>(line.m_points).m_x));
  }

  Quantity abs_distance_squared(Quantity x1, Quantity y1, Quantity x2,
      Quantity y2) {
    auto distance = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
    if(distance < Quantity(0)) {
      return -distance;
    }
    return distance;
  }

  bool within_interval(Quantity value, Quantity interval_start,
      Quantity interval_end, Quantity threshold) {
    if(interval_start < interval_end) {
      return interval_start - threshold <= value && value <=
        interval_end + threshold;
    }
    return interval_start + threshold >= value && value >=
      interval_end - threshold;
  }

  bool within_interval(Quantity value, Quantity interval_start,
      Quantity interval_end) {
    return within_interval(value, interval_start, interval_end, Quantity(0));
  }

  Quantity y_intercept(Quantity x, Quantity y, Quantity slope) {
    return y - (x * slope);
  }

  Quantity calculate_y(Quantity m, Quantity x, Quantity b) {
    return (m * x) + b;
  }
}

TrendLineModel::TrendLineModel()
  : m_last_id(0) {}

int TrendLineModel::add(const TrendLine& line) {
  m_trend_lines.push_back({line, ++m_last_id, State::UNSELECTED});
  return m_last_id;
}

TrendLine TrendLineModel::get(int id) {
  auto iter = find_id(id);
  if(iter == m_trend_lines.end()) {
    return TrendLine();
  }
  return (*iter).m_trend_line;
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

int TrendLineModel::intersects(const ChartPoint& point,
    ChartValue threshold) const {
  auto closest_id = -1;
  auto closest_distance = std::numeric_limits<Quantity>::infinity();
  auto threshold_qty = static_cast<Quantity>(threshold);
  auto threshold_squared = threshold_qty * threshold_qty;
  auto point_x = static_cast<Quantity>(point.m_x);
  auto point_y = static_cast<Quantity>(point.m_y);
  for(auto& line : m_trend_lines) {
    // TODO: edge cases for horizontal and verticals lines
    auto line_slope = slope(line.m_trend_line);
    auto line_b = y_intercept(
      static_cast<Quantity>(std::get<0>(line.m_trend_line.m_points).m_x),
      static_cast<Quantity>(std::get<0>(line.m_trend_line.m_points).m_y),
      line_slope);
    auto line_point_x =
      (point_x + (line_slope * point_y) - (line_slope * line_b)) /
      ((line_slope * line_slope) + 1);
    if(within_interval(line_point_x,
        static_cast<Quantity>(std::get<0>(line.m_trend_line.m_points).m_x),
        static_cast<Quantity>(std::get<1>(line.m_trend_line.m_points).m_x))) {
      auto distance_squared = abs_distance_squared(point_x, point_y,
        line_point_x, calculate_y(line_slope, line_point_x, line_b));
      if(distance_squared <= threshold_squared) {
        if(distance_squared < closest_distance) {
          closest_id = line.m_id;
          closest_distance = distance_squared;
        }
      }
    } // TODO: else if distance to either line point is <= threshold
  }
  return closest_id;
}

void TrendLineModel::remove(int id) {
  auto iter = find_id(id);
  if(iter != m_trend_lines.end()) {
    m_trend_lines.erase(iter);
  }
}

void TrendLineModel::set_selected(int id) {
  auto iter = find_id(id);
  if(iter != m_trend_lines.end()) {
    (*iter).m_state = State::SELECTED;
  }
}

void TrendLineModel::unset_selected(int id) {
  auto iter = find_id(id);
  if(iter != m_trend_lines.end()) {
    (*iter).m_state = State::UNSELECTED;
  }
}

void TrendLineModel::update(const TrendLine& line, int id) {
  auto iter = find_id(id);
  if(iter != m_trend_lines.end()) {
    (*iter).m_trend_line = line;
  }
}

connection TrendLineModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

std::vector<TrendLineModel::TrendLineData>::iterator
TrendLineModel::find_id(int id) {
  return std::find_if(m_trend_lines.begin(), m_trend_lines.end(),
    [&] (auto& line) {
      return id == line.m_id;
    });
}

void TrendLineModel::set_selected_status(int id, State state) {
  auto iter = find_id(id);
  if(iter != m_trend_lines.end()) {
    (*iter).m_state = state;
  }
}
