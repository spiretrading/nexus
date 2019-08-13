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
  return load_from_cache({first, last, first, last, limit});
}

connection CachedChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_chart_model->connect_candlestick_slot(slot);
}

bool CachedChartModel::is_set(RangeType a, RangeType b) {
  return static_cast<unsigned char>(a) & static_cast<unsigned char>(b);
}

bool CachedChartModel::is_in_range(ChartValue start, ChartValue end,
    const ChartRange& range) {
  if(range.m_type == RangeType::CLOSED) {
    return range.m_start <= start && end <= range.m_end;
  } else if(range.m_type == RangeType::OPEN) {
    return range.m_start < start && end < range.m_end;
  } else if(is_set(range.m_type, RangeType::LEFT_CLOSED)) {
    return range.m_start <= start && end < range.m_end;
  }
  return range.m_start < start && end <= range.m_end;
}

bool CachedChartModel::is_before_range(ChartValue value,
    const ChartRange& range) {
  if(is_set(range.m_type, RangeType::LEFT_CLOSED)) {
    return value <= range.m_start;
  }
  return value < range.m_start;
}

bool CachedChartModel::is_after_range(ChartValue value,
    const ChartRange& range) {
  if(is_set(range.m_type, RangeType::RIGHT_CLOSED)) {
    return value >= range.m_end;
  }
  return value > range.m_end;
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_from_cache(
    const LoadInfo& info) {
  return m_cache.load(info.m_requested_first, info.m_requested_last,
      info.m_limit).then(
    [=] (auto result) {
      auto load_first = info.m_requested_first;
      auto load_last = info.m_requested_last;
      for(auto& range : m_ranges) {
        if(is_in_range(info.m_requested_first, info.m_requested_last, range) ||
            result.Get().size() == info.m_limit.GetSize()) {
          return QtPromise<std::vector<Candlestick>>(
            [data = std::move(result.Get())] () mutable {
              return std::move(data);
            });
        }
        if(is_in_range(load_first, load_first, range)) {
          load_first = range.m_end;
        }
        if(is_before_range(load_first, range)) {
          load_last = min(load_last, range.m_start);
          break;
        }
      }
      return load_from_model({load_first, load_last, info.m_requested_first,
        info.m_requested_last, info.m_limit});
    });
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_from_model(
    const LoadInfo& info) {
  return m_chart_model->load(info.m_first, info.m_last, info.m_limit).then(
    [=] (auto result) {
      on_data_loaded(std::move(result.Get()),
        {info.m_first, info.m_last, RangeType::OPEN}, info.m_limit);
      return load_from_cache(info);
    });
}

void CachedChartModel::on_data_loaded(const std::vector<Candlestick>& data,
    const ChartRange& loaded_range) {
  auto first_iterator = data.begin();
  auto last_iterator = data.end();
  if(!data.empty()) {
    for(auto& range : m_ranges) {
      if(is_in_range(loaded_range.m_start, loaded_range.m_end, range)) {
        return;
      }
      if(is_in_range(loaded_range.m_start, loaded_range.m_start,
          {range.m_start, range.m_end, RangeType::CLOSED})) {
        first_iterator = std::upper_bound(data.begin(), data.end(),
          range.m_end,
          [] (const auto& value, const auto& index) {
            return index.GetStart() >= value;
          });
        if(first_iterator != data.end()) {
          ++first_iterator;
        }
      }
      if(is_in_range(loaded_range.m_end, loaded_range.m_end,
          {range.m_start, range.m_end, RangeType::CLOSED})) {
        last_iterator = std::lower_bound(data.begin(), data.end(),
          range.m_start,
          [] (const auto& index, const auto& value) {
            return index.GetEnd() <= value;
          });
        if(data.size() > 1 && first_iterator != data.end()) {
          --last_iterator;
        }
      }
    }
  }
  m_cache.store(std::move(
    std::vector<Candlestick>(first_iterator, last_iterator)));
  update_ranges(loaded_range);
}

void CachedChartModel::on_data_loaded(const std::vector<Candlestick>& data,
    const ChartRange& loaded_range, const SnapshotLimit& limit) {
  if(static_cast<int>(data.size()) < limit.GetSize()) {
    on_data_loaded(data, {loaded_range.m_start, loaded_range.m_end,
      RangeType::CLOSED});
  } else if(limit.GetType() == SnapshotLimit::Type::HEAD) {
    on_data_loaded(data, {loaded_range.m_start,
      max(data.back().GetStart(), loaded_range.m_start), RangeType::OPEN});
  } else {
    on_data_loaded(data, {min(data.front().GetEnd(), loaded_range.m_end),
      loaded_range.m_end, RangeType::OPEN});
  }
}

void CachedChartModel::update_ranges(const ChartRange& new_range) {
  if(m_ranges.empty()) {
    m_ranges.push_back(new_range);
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
  auto new_first = new_range.m_start;
  auto new_last = new_range.m_end;
  for(auto& range : m_ranges) {
    if(range.m_end < new_range.m_start) {
      continue;
    }
    if(range.m_start > new_range.m_end) {
      break;
    }
    if(range.m_start <= new_range.m_start && range.m_end >= new_range.m_start) {
      new_first = range.m_start;
      remove_range(ranges, range);
    } else if(range.m_start <= new_range.m_end &&
        range.m_end >= new_range.m_end) {
      new_last = range.m_end;
      remove_range(ranges, range);
    }
    if(range.m_start >= new_range.m_start && range.m_end <= new_range.m_end) {
      remove_range(ranges, range);
    }
  }
  auto index = std::lower_bound(ranges.begin(), ranges.end(), new_range.m_start,
    [] (const auto& index, const auto& value) {
      return index.m_start < value;
    });
  ranges.insert(index, {new_first, new_last, new_range.m_type});
  m_ranges = ranges;
}
