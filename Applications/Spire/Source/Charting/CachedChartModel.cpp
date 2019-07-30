#include "Spire/Charting/CachedChartModel.hpp"

using namespace Beam::Queries;
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
    ChartValue last, const SnapshotLimit& limit) {
  for(auto& range : m_ranges)  {
    if(range.m_start <= first && range.m_end >= last) {
      return QtPromise([=] {
        auto first_iterator = std::lower_bound(m_loaded_data.begin(),
          m_loaded_data.end(), first,
          [] (const auto& index, const auto& value) {
            return index.GetEnd() < value;
          });
        auto last_iterator = std::lower_bound(m_loaded_data.begin(),
          m_loaded_data.end(), last,
          [] (const auto& index, const auto& value) {
            return index.GetStart() < value;
          });
        if(last_iterator != m_loaded_data.end()) {
          ++last_iterator;
        }
        return std::vector<Candlestick>(first_iterator, last_iterator);
      });
    }
  }
  return load_data(get_gaps(first, last), first, last);
}

connection CachedChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_chart_model->connect_candlestick_slot(slot);
}

std::vector<CachedChartModel::ChartRange> CachedChartModel::get_gaps(
    ChartValue first, ChartValue last) {
  if(m_ranges.empty()) {
    return {{ChartRange{first, last}}};
  }
  auto range_points = std::vector<ChartValue>({first, last});
  for(auto& range : m_ranges) {
    if(range.m_end < first) {
      continue;
    }
    if(range.m_start >= last) {
      break;
    }
    if(range.m_start <= first && range.m_end >= first) {
      range_points.front() = range.m_end;
    } else if(range.m_start <= last && range.m_end >= last) {
      range_points.back() = range.m_start;
    } else {
      range_points.insert(range_points.end() - 1, range.m_start);
      range_points.insert(range_points.end() - 1, range.m_end);
    }
  }
  auto gaps = std::vector<ChartRange>();
  for(auto i = std::size_t(0); i < range_points.size(); i += 2) {
    gaps.push_back({range_points[i], range_points[i + 1]});
  }
  return gaps;
}

void CachedChartModel::insert_data(const std::vector<Candlestick>& data) {
  if(data.empty()) {
    return;
  }
  auto first = std::lower_bound(m_loaded_data.begin(),
    m_loaded_data.end(), data.front().GetStart(), [] (const auto& index,
        const auto& value) {
      return index.GetStart() < value;
    });
  auto last = std::lower_bound(m_loaded_data.begin(),
    m_loaded_data.end(), data.back().GetEnd(), [] (const auto& index,
        const auto& value) {
      return index.GetStart() < value;
    });
  auto index = m_loaded_data.erase(first, last);
  m_loaded_data.insert(index, data.begin(), data.end());
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_data(
    const std::vector<ChartRange>& gaps, ChartValue first, ChartValue last) {
  auto promises = std::vector<QtPromise<std::vector<Candlestick>>>();
  for(auto& gap : gaps) {
    promises.push_back(m_chart_model->load(gap.m_start, gap.m_end,
      SnapshotLimit::Unlimited()));
  }
  return all(std::move(promises)).then(
    [=] (std::vector<std::vector<Candlestick>> result) {
      for(auto i = std::size_t(0); i < gaps.size(); ++i) {
        on_data_loaded(result[i], gaps[i].m_start, gaps[i].m_end);
      }
      auto first_index = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), first,
        [] (const auto& index, const auto& value) {
          return index.GetEnd() < value;
        });
      auto last_index = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), last,
        [] (const auto& index, const auto& value) {
          return index.GetStart() < value;
        });
      if(last_index != m_loaded_data.end()) {
        ++last_index;
      }
      return std::vector<Candlestick>(first_index, last_index);
    });
}

void CachedChartModel::on_data_loaded(const std::vector<Candlestick>& data,
    ChartValue first, ChartValue last) {
  for(auto& range : m_ranges) {
    if(range.m_start <= first && range.m_end >= last) {
      return;
    }
  }
  insert_data(data);
  update_ranges(first, last);
}

void CachedChartModel::update_ranges(ChartValue first, ChartValue last) {
  if(m_ranges.empty()) {
    m_ranges.push_back({first, last});
    return;
  }
  auto remove_range = [] (auto& ranges, auto& range) {
    for(auto iter = ranges.begin(); iter != ranges.end(); ++iter) {
      if((*iter).m_start == range.m_start && (*iter).m_end == range.m_end) {
        ranges.erase(iter);
        break;
      }
    }
  };
  auto ranges = m_ranges;
  auto new_first = first;
  auto new_last = last;
  for(auto& range : m_ranges) {
    if(range.m_end < first) {
      continue;
    }
    if(range.m_start > last) {
      break;
    }
    if(range.m_start <= first && range.m_end >= first) {
      new_first = range.m_start;
      remove_range(ranges, range);
    } else if(range.m_start <= last && range.m_end >= last) {
      new_last = range.m_end;
      remove_range(ranges, range);
    }
    if(range.m_start >= first && range.m_end <= last) {
      remove_range(ranges, range);
    }
  }
  auto index = std::lower_bound(ranges.begin(), ranges.end(), first,
    [] (const auto& index, const auto& value) {
      return index.m_start < value;
    });
  ranges.insert(index, ChartRange{new_first, new_last});
  m_ranges = ranges;
}
