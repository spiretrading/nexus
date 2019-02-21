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
  for(auto& range : m_ranges) {
    if(range.m_start >= first && range.m_end <= last) {
      auto first_iterator = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), first);
      auto last_iterator = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), last);
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
