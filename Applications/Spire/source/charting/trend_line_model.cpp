#include "spire/charting/trend_line_model.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  ChartValue slope(const TrendLine& line) {
    if(std::get<0>(line.m_points).m_x < std::get<1>(line.m_points).m_x) {
      return ChartValue(
        (std::get<1>(line.m_points).m_y - std::get<0>(line.m_points).m_y)
        / (std::get<1>(line.m_points).m_x - std::get<0>(line.m_points).m_x));
    }
    return ChartValue(
      (std::get<0>(line.m_points).m_y - std::get<1>(line.m_points).m_y)
      / (std::get<0>(line.m_points).m_x - std::get<1>(line.m_points).m_x));
  }

  ChartValue distance_squared(const ChartPoint& point, const TrendLine& line) {
    auto line_slope = slope(line);
    auto line_point = std::get<0>(line.m_points);
    auto numerator = (static_cast<Nexus::Quantity>(line_slope) * point.m_x) -
      point.m_y + line_point.m_y -
      (static_cast<Nexus::Quantity>(line_slope) * line_point.m_x);
    numerator = static_cast<Nexus::Quantity>(numerator) * numerator;
    static_cast<double>(static_cast<Nexus::Quantity>(numerator));
    return ChartValue(numerator /
      ((static_cast<Nexus::Quantity>(line_slope) * line_slope) +
      ChartValue(1)));
  }

  bool within_range(ChartValue value, ChartValue range_value1,
      ChartValue range_value2, ChartValue threshold) {
    if(range_value1 < range_value2) {
      return range_value1 - threshold <= value && value <=
        range_value2 + threshold;
    }
    return range_value1 + threshold >= value && value >=
      range_value2 - threshold;
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
        std::get<1>(line.m_trend_line.m_points).m_y, threshold)) {
      auto distance = distance_squared(point, line.m_trend_line);
      if(distance < ChartValue(0)) {
        distance = -distance;
      }
      if(distance <= threshold_squared) {
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
