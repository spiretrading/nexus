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
    range, [] (const auto& index, const auto& value) {
      return value.m_start < index.m_start;
    });
  if(range_iterator != m_ranges.end() && (*range_iterator).m_start <= first &&
      (*range_iterator).m_end >= last) {
    return QtPromise([=] {
      auto first_iterator = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), first,
        [] (const auto& index, const auto& value) {
          return index.GetStart() < value;
      });
      auto last_iterator = std::upper_bound(m_loaded_data.begin(),
        m_loaded_data.end(), last,
        [] (const auto& value, const auto& index) {
          return index.GetEnd() > value;
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
  m_ranges;
  auto a = m_chart_model->load(data.m_start, data.m_end).then(
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
      auto pos = m_loaded_data.end();
      if(last != m_loaded_data.end()) {
        pos = m_loaded_data.erase(first, last + 1);
      } else {
        pos = m_loaded_data.erase(first, last);
      }
      m_loaded_data.insert(pos, result.Get().begin(), result.Get().end());
      auto [range_first, range_last] = range_search(data);
      auto new_range = ChartRange{
        [&, range_first = range_first] {
          if(range_first == m_ranges.end()) {
            return data.m_start;
          }
          return std::min((*range_first).m_start, data.m_start);
        }(),
        [&, range_last = range_last] {
          if(range_last == m_ranges.end()) {
            return data.m_end;
          }
          return std::max((*range_last).m_end, data.m_end);
        }()};
      auto range_pos = m_ranges.erase(range_first, range_last);
      m_ranges.insert(range_pos, data);
      return result.Get();
  });
  m_loaded_data;
  return a;
}

std::tuple<std::vector<CachedChartModel::ChartRange>::iterator,
    std::vector<CachedChartModel::ChartRange>::iterator>
    CachedChartModel::range_search(const ChartRange& range) {
  if(m_ranges.empty()) {
    return {m_ranges.end(), m_ranges.end()};
  }
  auto first = std::optional<std::vector<ChartRange>::iterator>();
  auto last = std::optional<std::vector<ChartRange>::iterator>();
  if(range.m_start < m_ranges.front().m_start) {
    first = m_ranges.begin();
    if(range.m_end < m_ranges.front().m_start) {
      return {first.value(), first.value()};
    }
  }
  if(range.m_end > m_ranges.back().m_end) {
    last = m_ranges.end();
    if(range.m_start > m_ranges.back().m_end) {
      return {last.value(), last.value()};
    }
  }
  for(auto iter = m_ranges.begin(); iter != m_ranges.end(); ++iter) {
    if((*iter).m_start <= range.m_start &&
        (*iter).m_end >= range.m_start) {
      first = iter;
    }
    if((*iter).m_start <= range.m_end && (*iter).m_end >= range.m_end) {
      last = iter + 1;
    }
    if(!first.has_value() && range.m_start > (*iter).m_end &&
        range.m_start < (*(iter + 1)).m_start) {
      first = iter + 1;
    }
    if(!last.has_value() && range.m_end > (*iter).m_end &&
        range.m_end < (*(iter + 1)).m_start) {
      last = iter + 1;
    }
    if(first.has_value() && last.has_value()) {
      break;
    }
  }
  return {first.value(), last.value()};
}
