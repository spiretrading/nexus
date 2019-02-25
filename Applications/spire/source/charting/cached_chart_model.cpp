#include "spire/charting/cached_chart_model.hpp"

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
  auto range = ChartRange{first, last};
  auto range_iterator = std::lower_bound(m_ranges.begin(), m_ranges.end(),
    range, [] (const auto& value, const auto& search_value) {
      return value.m_start < search_value.m_start &&
        search_value.m_end < value.m_end;
    });
  if(range_iterator != m_ranges.end() && (*range_iterator).m_start <= first &&
      (*range_iterator).m_end >= last) {
    return QtPromise([=] {
      auto first_iterator = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), first,
        [] (const auto& value, const auto& search_value) {
          return value.GetStart() < search_value;
      });
      auto last_iterator = std::upper_bound(m_loaded_data.begin(),
        m_loaded_data.end(), last,
        [] (const auto& search_value, const auto& value) {
          return search_value < value.GetEnd();
      });
      return std::vector<Candlestick>(first_iterator, last_iterator);
    });
  }
  return load_data({first, last});
}

connection CachedChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_chart_model->connect_candlestick_slot(slot);
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_data(
    const ChartRange& data) {
  return m_chart_model->load(data.m_start, data.m_end).then(
    [=] (auto result) {
      auto first = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), data.m_start, [] (const auto& lhs,
            const auto& rhs) {
          return lhs.GetStart() < rhs;
        });
      auto last = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), data.m_end, [] (const auto& lhs,
            const auto& rhs) {
          return lhs.GetStart() < rhs;
        });
      auto pos = m_loaded_data.erase(first, last);
      m_loaded_data.insert(pos, result.Get().begin(), result.Get().end());
      auto range_first = std::lower_bound(m_ranges.begin(), m_ranges.end(),
        data, [] (const auto& lhs, const auto& rhs) {
          return lhs.m_start < rhs.m_start;
        });
      auto range_last = std::lower_bound(m_ranges.begin(), m_ranges.end(),
        data, [] (const auto& lhs, const auto& rhs) {
          return lhs.m_start < rhs.m_start;
        });
      auto range_pos = m_ranges.erase(range_first, range_last);
      m_ranges.insert(range_pos, data);
      for(auto iter = m_ranges.begin(); iter != m_ranges.end() - 1;) {
        if((*iter).m_end > (*(iter + 1)).m_start) {
          (*(iter + 1)).m_start = (*iter).m_start;
          iter = m_ranges.erase(iter);
        } else {
          ++iter;
        }
      }
      return result.Get();
  });
}
