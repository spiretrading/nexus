#include "spire/charting/cached_chart_model.hpp"
#include <stack>
#include <boost/range/adaptor/reversed.hpp>

using namespace boost::signals2;
using namespace Spire;

CachedChartModel::CachedChartModel(ChartModel& model)
    : m_chart_model(&model) {}

ChartValue::Type CachedChartModel::get_x_axis_type() const {
  return m_chart_model->get_x_axis_type();
}

ChartValue::Type CachedChartModel::get_y_axis_type() const {
  return m_chart_model->get_y_axis_type();
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load(ChartValue first,
    ChartValue last) {
  if(m_ranges.empty()) {
    return load_data({ChartRange({first, last})});
  }
  for(auto& range : m_ranges) {
    if(range.m_start >= first && range.m_end <= last) {
      //auto value_iterator = [=] (const auto& value) {
      //  return std::find_if(m_loaded_data.begin(), m_loaded_data.end(),
      //    [=] (const auto& v) { return v.GetEnd() >= value &&
      //    v.GetStart() <= value; });
      //};
      auto first_iterator = m_loaded_data.begin();
      auto last_iterator = m_loaded_data.end();
      for(auto it = m_loaded_data.begin(); it != m_loaded_data.end(); ++it) {
        qDebug() << static_cast<double>(static_cast<Nexus::Money>((*it).GetStart()));
        if((*it).GetStart() >= first && (*it).GetEnd() <= first) {
          first_iterator = it;
        } else if((*it).GetStart() >= last && (*it).GetEnd() <= last) {
          last_iterator = it;
        }
      }
      //auto first_iterator = value_iterator(first);
      //auto last_iterator = value_iterator(last);
      return make_qt_promise([=] {
        return std::vector<Candlestick>(first_iterator, last_iterator);
      });
    }
  }
  return load_data({{first, last}});
}

connection CachedChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_chart_model->connect_candlestick_slot(slot);
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_data(
    const std::vector<ChartRange>& data) {
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
        auto top = m_ranges.back();
        if(range.m_start <= top.m_end) {
          auto upper_bound = std::max(top.m_end, range.m_end);
          m_ranges.back() = {top.m_start, upper_bound};
        } else {
          m_ranges.push_back(range);
        }
      }
    }
  }
  return m_chart_model->load(data.front().m_start, data.front().m_end);
}
