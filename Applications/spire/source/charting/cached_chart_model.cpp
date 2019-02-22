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
  auto range = ChartRange({first, last});
  auto range_iterator = std::lower_bound(m_ranges.begin(), m_ranges.end(),
    range, [=] (const auto& value, const auto& search_value) {
      return value.m_start >= search_value.m_start &&
        value.m_end <= search_value.m_end;
    });
  if(range_iterator != m_ranges.end() && (*range_iterator).m_start <= first &&
      (*range_iterator).m_end >= last) {
    return QtPromise([=] {
      auto first_iterator = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), first,
        [=] (const auto& value, const auto& search_value) {
          return value.GetStart() < search_value;
      });
      auto last_iterator = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), last,
        [=] (const auto& value, const auto& search_value) {
          return value.GetEnd() <= search_value;
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
        m_loaded_data.end(), data.m_start, [=] (const auto& lhs,
            const auto& rhs) {
          return lhs.GetStart() < rhs;
        });
      auto last = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), data.m_end, [=] (const auto& lhs,
            const auto& rhs) {
          return lhs.GetStart() < rhs;
        });
      auto pos = m_loaded_data.erase(first, last);
      m_loaded_data.insert(pos, result.Get().begin(), result.Get().end());
      if(m_ranges.empty()) {
        m_ranges.push_back(data);
      } else {
        auto ranges = m_ranges;
        ranges.push_back(data);
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
      return result.Get();
  });
}
