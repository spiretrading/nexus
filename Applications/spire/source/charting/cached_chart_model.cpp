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
      auto first_iterator = m_loaded_data.begin();
      auto last_iterator = m_loaded_data.end();
      for(auto it = m_loaded_data.begin(); it != m_loaded_data.end(); ++it) {
        if((*it).GetStart() >= first && (*it).GetEnd() <= first) {
          first_iterator = it;
        } else if((*it).GetStart() >= last && (*it).GetEnd() <= last) {
          last_iterator = it;
        }
      }
      return make_qt_promise([=] {
        return std::vector<Candlestick>(first_iterator, last_iterator);
      });
    }
  }
  return load_data({first, last});
}

connection CachedChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_chart_model->connect_candlestick_slot(slot);
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_data(
    const ChartRange& data) {
  auto data_promise = m_chart_model->load(data.m_start, data.m_end);
  data_promise.then([=] (auto result) {
    // add data to loaded data
    // update range(s)
  });
  return data_promise;
}
