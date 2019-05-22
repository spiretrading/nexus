#include "spire/charting/trend_line_model.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  ChartValue slope(const TrendLine& line) {
    if(std::get<0>(line.m_points).m_x < std::get<1>(line.m_points).m_x) {
      return ChartValue(
        (std::get<1>(line.m_points).m_x - std::get<0>(line.m_points).m_x)
        / (std::get<1>(line.m_points).m_y - std::get<0>(line.m_points).m_y));
    }
    return ChartValue(
      (std::get<0>(line.m_points).m_x - std::get<1>(line.m_points).m_x)
      / (std::get<0>(line.m_points).m_y - std::get<1>(line.m_points).m_y));
  }

  ChartValue distance_squared(const ChartPoint& point, const TrendLine& line) {
    auto line_slope = slope(line);
    auto slope_squared = static_cast<Nexus::Quantity>(line_slope) * line_slope;
    auto point_squared = ChartPoint({static_cast<Nexus::Quantity>(point.m_x) *
      point.m_x, static_cast<Nexus::Quantity>(point.m_y) * point.m_y});
    auto line_point = std::get<0>(line.m_points);
    auto line_point_squared = ChartPoint({
      static_cast<Nexus::Quantity>(line_point.m_x) * line_point.m_x,
      static_cast<Nexus::Quantity>(line_point.m_y) * line_point.m_y});
    return ChartValue(
      ((static_cast<Nexus::Quantity>(slope_squared) * point_squared.m_x) -
      point_squared.m_y + line_point_squared.m_y -
      (static_cast<Nexus::Quantity>(slope_squared) * line_point_squared.m_x)) /
      (line_slope + ChartValue(1)));
  }

  bool within_range(ChartValue value, ChartValue range_value1,
      ChartValue range_value2) {
    if(range_value1 < range_value2) {
      return range_value1 <= value && value <= range_value2;
    }
    return range_value1 >= value && value >= range_value2;
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
  // TODO: How to multiply these without casting?
  auto threshold_squared = static_cast<Nexus::Quantity>(threshold) * threshold;
  for(auto& line : m_trend_lines) {
    if(within_range(point.m_y, std::get<0>(line.m_trend_line.m_points).m_y,
        std::get<1>(line.m_trend_line.m_points).m_y)) {
      if(distance_squared(point, line.m_trend_line) <= threshold_squared) {
        return line.m_id;
      }
    }
  }
  return -1;
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
