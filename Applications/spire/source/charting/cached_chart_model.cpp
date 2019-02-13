#include "spire/charting/cached_chart_model.hpp"

using namespace Spire;

CachedChartModel::CachedChartModel(ChartModel* model)
    : m_chart_model(model) {}

ChartValue::Type CachedChartModel::get_x_axis_type() const {
  return m_chart_model->get_x_axis_type();
}

ChartValue::Type CachedChartModel::get_y_axis_type() const {
  return m_chart_model->get_y_axis_type();
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load(ChartValue first,
    ChartValue last) {
  auto first_index = -1;
  auto last_index = -1;
  for(auto& range : m_ranges) {
    if(range.m_start >= first) {
      first_index = get_index(first);
    } else if(range.m_end <= last) {
      last_index = get_index(last);
      break;
    }
  }
  auto gaps = std::vector<ChartRange>();
  if(first_index == -1) {
    gaps.push_back({first, ChartValue()});
  }
  if(last_index == -1) {
    if(m_loaded_data.begin()->GetStart() > last) {
      gaps[0].m_end = last;
      // loop through gaps, load gaps
      // stitch everything together
      // return new promise
    }
  }
  if(gaps.begin() == gaps.end()) {
    // assumes m_ranges isn't empty
    gaps.push_back({m_ranges.front().m_end, ChartValue()});
  }
  for(auto i = 0; i < m_ranges.size(); ++i) {
    // we're in between two ranges
    if(last < m_ranges[i + 1].m_start) {
      gaps.back().m_end = last;
      break;
    }
    // otherwise, complete this gap
    gaps.back().m_end = m_ranges[i + 1].m_start;
  }
  if(gaps.back().m_end == ChartValue()) {
    gaps.back().m_end = last;
  }
  // loop through gaps, load gaps
  // stitch everything together
  // return new promise
}

int CachedChartModel::get_index(const ChartValue& value) {
  auto it = std::find(m_loaded_data.begin(), m_loaded_data.end(),
    value);
  if(it == m_loaded_data.end()) {
    return -1;
  }
  return it - m_loaded_data.begin();
}
