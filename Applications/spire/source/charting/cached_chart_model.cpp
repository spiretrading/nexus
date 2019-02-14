#include "spire/charting/cached_chart_model.hpp"
#include <stack>
#include <boost/range/adaptor/reversed.hpp>

using namespace boost::signals2;
using namespace Spire;

CachedChartModel::CachedChartModel(ChartModel* model)
    : m_chart_model(model) {
  m_ranges.push_back({ChartValue(1), ChartValue(4)});
  m_ranges.push_back({ChartValue(7), ChartValue(10)});
  m_ranges.push_back({ChartValue(14), ChartValue(17)});
  m_loaded_data = {
    Candlestick(ChartValue(1), ChartValue(2)),
    Candlestick(ChartValue(2), ChartValue(3)),
    Candlestick(ChartValue(3), ChartValue(4)),
    Candlestick(ChartValue(7), ChartValue(8)),
    Candlestick(ChartValue(8), ChartValue(9)),
    Candlestick(ChartValue(9), ChartValue(10)),
    Candlestick(ChartValue(14), ChartValue(15)),
    Candlestick(ChartValue(15), ChartValue(16)),
    Candlestick(ChartValue(16), ChartValue(17))};
}

ChartValue::Type CachedChartModel::get_x_axis_type() const {
  return m_chart_model->get_x_axis_type();
}

ChartValue::Type CachedChartModel::get_y_axis_type() const {
  return m_chart_model->get_y_axis_type();
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load(ChartValue first,
    ChartValue last) {
  if(m_ranges.empty()) {
    auto initial_data = ChartRange({first, last});
    return load_data({initial_data});
  }
  // remove this after testing
  first = ChartValue(-2);
  last = ChartValue(12);
  // don't actually need the index, just need to know if it's in the loaded data
  auto first_index = get_loaded_data_index(first);
  auto last_index = get_loaded_data_index(last);
  auto gaps = std::vector<ChartRange>();
  if(first_index == -1) {
    gaps.push_back({first, ChartValue()});
  }
  if(last_index == -1) {
    if(m_loaded_data.begin()->GetStart() > last || (first_index == -1 &&
        last_index == -1)) {
      gaps.front().m_end = last;
      // uncomment this after testing
      //return load_data(gaps);
    }
  }
  if(gaps.begin() == gaps.end()) {
    gaps.push_back({m_ranges.front().m_end, ChartValue()});
  }
  for(auto i = 0; i < m_ranges.size(); ++i) {
    if(last < m_ranges[i].m_start) {
      gaps.back().m_end = last;
      break;
    }
    gaps.back().m_end = m_ranges[i].m_start;
    if(m_ranges[i].m_end > last) {
      break;
    }
    gaps.push_back({m_ranges[i].m_end, ChartValue()});
  }
  if(gaps.back().m_end == ChartValue()) {
    gaps.back().m_end = last;
  }
  return load_data(gaps);
}

connection CachedChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_chart_model->connect_candlestick_slot(slot);
}

int CachedChartModel::get_loaded_data_index(const ChartValue& value) {
  if(m_loaded_data.front().GetStart() == value) {
    return 0;
  }
  auto it = std::find_if(m_loaded_data.begin(), m_loaded_data.end(),
    [&] (const auto& v) { return v.GetEnd() == value; });
  if(it == m_loaded_data.end()) {
    return -1;
  }
  return it - m_loaded_data.begin();
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_data(
    const std::vector<ChartRange>& data) {
  // load data
  // stitch everything together



  if(m_ranges.empty()) {
    m_ranges.push_back(data.front());
  } else {
    auto ranges = m_ranges;
    ranges.insert(ranges.end(), data.begin(), data.end());
    std::sort(ranges.begin(), ranges.end(),
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_start < rhs.m_start;
      });
    m_ranges.clear();
    for(auto& range : ranges) {
      if(m_ranges.empty()) {
        m_ranges.push_back(range);
      } else {
        auto lower = m_ranges.back();
        if(range.m_start <= lower.m_end) {
          auto upper_bound = std::max(lower.m_end, range.m_end);
          m_ranges.back() = {lower.m_start, upper_bound};
        } else {
          m_ranges.push_back(range);
        }
      }
    }
  }




  // return new promise
  return QtPromise<std::vector<Candlestick>>();
}
