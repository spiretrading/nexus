#include "spire/charting/trend_line_model.hpp"

using namespace boost::signals2;
using namespace Spire;

TrendLineModel::TrendLineModel()
  : m_last_id(0) {}

int TrendLineModel::add(const TrendLine& line) {
  m_trend_lines.push_back({line, m_last_id++, State::UNSELECTED});
}

TrendLine TrendLineModel::get(int id) const {
  auto iter = std::find_if(m_trend_lines.begin(), m_trend_lines.end(),
    [&] (auto& line) {
      return id == std::get<1>(line);
    });
  if(iter == m_trend_lines.end()) {
    return TrendLine();
  }
  return std::get<0>(*iter);
}

std::vector<TrendLine> TrendLineModel::get_lines() const {
  auto lines = std::vector<TrendLine>();
  for(auto& line : m_trend_lines) {
    lines.push_back(std::get<0>(line));
  }
  return lines;
}

std::vector<int> TrendLineModel::get_selected() const {
  auto selected = std::vector<int>();
  for(auto& line : m_trend_lines) {
    selected.push_back(std::get<1>(line));
  }
  return selected;
}

int TrendLineModel::intersects(const ChartPoint& point) const {

}

void TrendLineModel::remove(int id) {
  auto iter = std::find_if(m_trend_lines.begin(), m_trend_lines.end(),
    [&] (auto& line) {
      return id == std::get<1>(line);
    });
  if(iter != m_trend_lines.end()) {
    m_trend_lines.erase(iter);
  }
}

void TrendLineModel::set_selected(int id) {
  auto iter = std::find_if(m_trend_lines.begin(), m_trend_lines.end(),
    [&] (auto& line) {
      return id = std::get<1>(line);
    });
  if(iter != m_trend_lines.end()) {
    std::get<2>(*iter) = State::SELECTED;
  }
}

void TrendLineModel::unset_selected(int id) {
  auto iter = std::find_if(m_trend_lines.begin(), m_trend_lines.end(),
    [&] (auto& line) {
      return id == std::get<1>(line);
    });
  if(iter != m_trend_lines.end()) {
    std::get<2>(*iter) = State::UNSELECTED;
  }
}

void TrendLineModel::update(const TrendLine& line, int id) {
  auto iter = std::find_if(m_trend_lines.begin(), m_trend_lines.end(),
    [&] (auto& line) {
      return id == std::get<1>(line);
    });
  if(iter != m_trend_lines.end()) {
    std::get<0>(*iter) = line;
  }
}

connection TrendLineModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

void TrendLineModel::set_selected_status(int id, State state) {
  auto iter = std::find_if(m_trend_lines.begin(), m_trend_lines.end(),
    [&] (auto& line) {
      return id == std::get<1>(line);
    });
  if(iter != m_trend_lines.end()) {
    std::get<2>(*iter) = state;
  }
}
