#include "Spire/Charting/CachedChartModel.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Beam::Queries;
using namespace boost::signals2;
using namespace Spire;

CachedChartModel::CachedChartModel(ChartModel& model)
    : m_chart_model(&model),
      m_cache(model.get_x_axis_type(), model.get_y_axis_type(), {}) {}

ChartValue::Type CachedChartModel::get_x_axis_type() const {
  return m_chart_model->get_x_axis_type();
}

ChartValue::Type CachedChartModel::get_y_axis_type() const {
  return m_chart_model->get_y_axis_type();
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load(ChartValue first,
    ChartValue last, const SnapshotLimit& limit) {
  return load_from_cache(first, last, first, last, limit);
}

connection CachedChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_chart_model->connect_candlestick_slot(slot);
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_from_cache(
    ChartValue first, ChartValue last, ChartValue requested_first,
    ChartValue requested_last, const SnapshotLimit& limit) {
  return m_cache.load(requested_first, requested_last, limit).then(
    [=] (auto result) {
      auto load_first = first;
      auto load_last = last;
      for(auto& range : m_ranges) {
        if(range.m_start <= requested_first && requested_last <= range.m_end) {
          return QtPromise<std::vector<Candlestick>>(
            [data = std::move(result.Get())] () { 
              return data; });
        }
        if(range.m_end < first) {
          continue;
        }
        if(range.m_start >= load_last) {
          break;
        }
        if(range.m_start <= load_first && load_first <= range.m_end) {
          load_first = range.m_end;
        } else if(range.m_start <= load_last && load_last <= range.m_end) {
          load_last = range.m_start;
        }
      }
      return load_from_model(load_first, load_last, requested_first,
        requested_last, limit);
    });
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_from_model(
    ChartValue first, ChartValue last, ChartValue requested_first,
    ChartValue requested_last,const SnapshotLimit& limit) {
  return m_chart_model->load(first, last, limit).then([=] (auto result) {
      auto data_size = static_cast<int>(result.Get().size());
      on_data_loaded(std::move(result.Get()), first, last, limit);
      return load_from_cache(first, last, requested_first, requested_last,
        limit);
    });
}

void CachedChartModel::on_data_loaded(const std::vector<Candlestick>& data,
    ChartValue first, ChartValue last) {
  for(auto& range : m_ranges) {
    if(range.m_start <= first && range.m_end >= last) {
      return;
    }
  }
  auto sticks = data;
  auto front = std::move(wait(m_cache.load(data.front().GetEnd(),
    data.front().GetEnd(), SnapshotLimit::FromHead(1))));
  auto back = std::move(wait(m_cache.load(data.back().GetStart(),
    data.back().GetStart(), SnapshotLimit::FromHead(1))));
  // TODO: this will fail if there a multiple values that end the given end.
  //        also write a test for it.
  if(!front.empty() && sticks.front() == front.front()) {
    sticks.erase(sticks.begin());
  }
  if(!back.empty() && sticks.back() == back.front()) {
    sticks.pop_back();
  }
  m_cache.store(std::move(sticks));
  update_ranges(first, last);
}

void CachedChartModel::on_data_loaded(const std::vector<Candlestick>& data,
    ChartValue first, ChartValue last, const SnapshotLimit& limit) {
  if(static_cast<int>(data.size()) < limit.GetSize()) {
    on_data_loaded(data, first, last);
  } else if(limit.GetType() == SnapshotLimit::Type::HEAD) {
    on_data_loaded(data, first, data.back().GetStart());
  } else {
    on_data_loaded(data, data.front().GetEnd(), last);
  }
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
