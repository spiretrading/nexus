#include "Spire/Charting/CachedChartModel.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Beam::Queries;
using namespace boost::icl;
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

QtPromise<std::vector<Candlestick>> CachedChartModel::load_from_cache(
    const LoadInfo& info) {
  return m_cache.load(info.m_requested_first, info.m_requested_last,
      info.m_limit).then(
    [=] (auto result) {
      if(contains(m_ranges, continuous_interval<ChartValue>::closed(
          info.m_requested_first, info.m_requested_last))) {
        return QtPromise<std::vector<Candlestick>>(
          [data = std::move(result.Get())] () mutable {
            return std::move(data);
          });
      }
      auto load_first = info.m_requested_first;
      auto load_last = info.m_requested_last;
      for(auto i = m_ranges.begin(); i != m_ranges.end(); ++i) {
        
      }
      for(auto range : m_ranges) {
        auto first = continuous_interval<ChartValue>::closed(
          load_first, load_first);
        if(intersects(first, range)) {
          load_first = range.upper();
        }
        //if(is_before_range(load_first, range)) {
        //  load_last = min(load_last, range.lower());
        //  break;
        //}
        if(exclusive_less(first, range)) {
          load_last = min(load_last, range.lower());
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
        continuous_interval<ChartValue>::open(info.m_first, info.m_last),
        info.m_limit);
      return load_from_cache(info);
    });
}

void CachedChartModel::on_data_loaded(const std::vector<Candlestick>& data,
    const continuous_interval<ChartValue>& loaded_range) {
  if(contains(m_ranges, loaded_range)) {
    return;
  }
  auto first_iterator = data.begin();
  auto last_iterator = data.end();
  if(!data.empty()) {
    for(auto& range : m_ranges) {
      auto first = continuous_interval<ChartValue>::closed(
        loaded_range.lower(), loaded_range.lower());
      if(intersects(first, range)) {
        first_iterator = std::upper_bound(data.begin(), data.end(),
          range.upper(),
          [] (const auto& value, const auto& index) {
            return index.GetStart() >= value;
          });
        if(first_iterator != data.end()) {
          ++first_iterator;
        }
      }
      auto last = continuous_interval<ChartValue>::closed(
        loaded_range.upper(), loaded_range.upper());
      if(intersects(last, range)) {
        last_iterator = std::lower_bound(data.begin(), data.end(),
          range.lower(),
          [] (const auto& index, const auto& value) {
            return index.GetEnd() <= value;
          });
        if(data.size() > 1 && first_iterator < last_iterator) {
          --last_iterator;
        }
      }
    }
  }
  m_cache.store(std::move(
    std::vector<Candlestick>(first_iterator, last_iterator)));
  m_ranges.add(loaded_range);
}

void CachedChartModel::on_data_loaded(const std::vector<Candlestick>& data,
    const continuous_interval<ChartValue>& loaded_range,
    const SnapshotLimit& limit) {
  if(static_cast<int>(data.size()) < limit.GetSize()) {
    on_data_loaded(data, continuous_interval<ChartValue>::closed(
      loaded_range.lower(), loaded_range.upper()));
  } else if(limit.GetType() == SnapshotLimit::Type::HEAD) {
    auto range = [&] () {
        if(loaded_range.lower() < loaded_range.upper()) {
          return continuous_interval<ChartValue>::right_open(
            loaded_range.lower(),
            max(data.back().GetStart(), loaded_range.lower()));
        }
        return continuous_interval<ChartValue>::open(
          loaded_range.lower(), loaded_range.upper());
      }();
    on_data_loaded(data, range);
  } else {
    auto range = [&] () {
        if(loaded_range.lower() < loaded_range.upper()) {
          return continuous_interval<ChartValue>::left_open(
            loaded_range.lower(), loaded_range.upper());
        }
        return continuous_interval<ChartValue>::open(
          loaded_range.lower(), loaded_range.upper());
      }();
    on_data_loaded(data, range);
  }
}
