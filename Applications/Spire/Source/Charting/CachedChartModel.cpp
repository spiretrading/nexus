#include "Spire/Charting/CachedChartModel.hpp"
#include <algorithm>
#include <boost/range/adaptor/reversed.hpp>

using namespace Beam::Queries;
using namespace boost;
using namespace boost::adaptors;
using namespace boost::icl;
using namespace boost::signals2;
using namespace Spire;

CachedChartModel::CachedChartModel(ChartModel& model)
  : m_chart_model(&model),
    m_cache(model.get_x_axis_type(), model.get_y_axis_type(), {}) {}

Scalar::Type CachedChartModel::get_x_axis_type() const {
  return m_chart_model->get_x_axis_type();
}

Scalar::Type CachedChartModel::get_y_axis_type() const {
  return m_chart_model->get_y_axis_type();
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load(Scalar first,
    Scalar last, const SnapshotLimit& limit) {
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
      if(static_cast<int>(result.Get().size()) == info.m_limit.GetSize() ||
          contains(m_ranges, continuous_interval<Scalar>::closed(
          info.m_requested_first, info.m_requested_last))) {
        return QtPromise(std::move(result.Get()));
      }
      auto load_first = info.m_requested_first;
      auto load_last = info.m_requested_last;
      if(info.m_limit.GetType() == SnapshotLimit::Type::HEAD) {
        for(auto& range : m_ranges) {
          auto first = continuous_interval<Scalar>::closed(load_first,
            load_first);
          if(intersects(first, range)) {
            load_first = range.upper();
          }
          if(exclusive_less(first, range)) {
            load_last = std::min(load_last, range.lower());
            break;
          }
        }
      } else {
        for(auto& range : reverse(m_ranges)) {
          auto last = continuous_interval<Scalar>::closed(load_last, load_last);
          if(last > range) {
            load_first = std::max(load_first, range.upper());
            break;
          }
          if(intersects(last, range)) {
            load_last = range.lower();
          }
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
      if(info.m_requested_first == info.m_requested_last &&
          info.m_limit.GetSize() >= static_cast<int>(result.Get().size())) {
        return QtPromise(std::move(result.Get()));
      }
      on_data_loaded(std::move(result.Get()), info);
      return load_from_cache(info);
    });
}

void CachedChartModel::on_data_loaded(const std::vector<Candlestick>& data,
    const continuous_interval<Scalar>& loaded_range) {
  if(contains(m_ranges, loaded_range)) {
    return;
  }
  auto first_iterator = data.begin();
  auto last_iterator = data.end();
  if(!data.empty()) {
    for(auto& range : m_ranges) {
      auto first = continuous_interval<Scalar>::closed(loaded_range.lower(),
        loaded_range.lower());
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
      auto last = continuous_interval<Scalar>::closed(loaded_range.upper(),
        loaded_range.upper());
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
  m_cache.store({first_iterator, last_iterator});
  m_ranges.add(loaded_range);
}

void CachedChartModel::on_data_loaded(const std::vector<Candlestick>& data,
    const LoadInfo& info) {
  if(static_cast<int>(data.size()) < info.m_limit.GetSize()) {
    on_data_loaded(data, continuous_interval<Scalar>::closed(info.m_first,
      info.m_last));
  } else if(info.m_limit.GetType() == SnapshotLimit::Type::HEAD) {
    auto range = continuous_interval<Scalar>::right_open(info.m_first,
      std::min(data.back().GetEnd(), info.m_last));
    on_data_loaded(data, range);
  } else {
    auto range = continuous_interval<Scalar>::left_open(
      std::max(data.front().GetStart(), info.m_first), info.m_last);
    on_data_loaded(data, range);
  }
}
