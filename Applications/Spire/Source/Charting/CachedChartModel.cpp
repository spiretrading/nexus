#include "Spire/Charting/CachedChartModel.hpp"
#include <boost/range/adaptor/reversed.hpp>
#include <boost/icl/continuous_interval.hpp>

using namespace Beam::Queries;
using namespace boost::adaptors;
using namespace boost::icl;
using namespace boost::signals2;
using namespace Spire;

namespace {
  std::size_t count_end_overlapping_candlesticks(
      const std::vector<Candlestick>& candlesticks, Scalar end) {
    auto result = std::size_t(0);
    for(auto& candlestick : reverse(candlesticks)) {
      if(end <= candlestick.GetEnd()) {
        ++result;
      } else {
        break;
      }
    }
    return result;
  }

  std::size_t count_start_overlapping_candlesticks(Scalar start,
      const std::vector<Candlestick>& candlesticks) {
    auto result = std::size_t(0);
    for(auto& candlestick : candlesticks) {
      if(candlestick.GetStart() <= start) {
        ++result;
      } else {
        break;
      }
    }
    return result;
  }
}

CachedChartModel::CachedChartModel(ChartModel& model)
  : m_model(&model),
    m_cache(model.get_x_axis_type(), model.get_y_axis_type(), {}) {}

Scalar::Type CachedChartModel::get_x_axis_type() const {
  return m_cache.get_x_axis_type();
}

Scalar::Type CachedChartModel::get_y_axis_type() const {
  return m_cache.get_y_axis_type();
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load(Scalar first,
    Scalar last, const SnapshotLimit& limit) {
  if(limit.GetSize() == 0) {
    return QtPromise(std::vector<Candlestick>());
  }
  auto interval = [&] {
    if(first == last) {
      return Interval::closed(first, last);
    } else {
      return Interval::open(first, last);
    }
  }();
  auto [bound, cached_interval, queried_interval] = [&] {
    if(limit.GetType() == SnapshotLimit::Type::HEAD) {
      return std::tuple(first, find_leftmost_cached_overlap(interval),
        find_leftmost_queried_overlap(interval));
    } else {
      return std::tuple(last, find_rightmost_cached_overlap(interval),
        find_rightmost_queried_overlap(interval));
    }
  }();
  if(cached_interval && contains(*cached_interval, bound)) {
    return load_cached_interval(interval, *cached_interval, limit);
  } else if(queried_interval && contains(*queried_interval, bound)) {
    return load_queried_interval(interval, *queried_interval, limit);
  } else {
    return load_new_interval(interval, cached_interval, queried_interval,
      limit);
  }
}

connection CachedChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_model->connect_candlestick_slot(slot);
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_cached_interval(
    const Interval& interval, const Interval& cached_interval,
    const SnapshotLimit& limit) {
  auto is_head = limit.GetType() == SnapshotLimit::Type::HEAD;
  auto [start, end] = [&] {
    if(is_head) {
      return std::tuple(interval.lower(), std::min(interval.upper(),
        cached_interval.upper()));
    } else {
      return std::tuple(std::max(interval.lower(), cached_interval.lower()),
        interval.upper());
    }
  }();
  return m_cache.load(start, end, limit).then(
    [=] (auto result) {
      auto candlesticks = std::move(result.Get());
      if((is_head && end == interval.upper()) || (
          !is_head && start == interval.lower()) ||
          candlesticks.size() == limit.GetSize()) {
        return QtPromise(std::move(candlesticks));
      } else {
        auto [new_start, new_end] = [&] {
          if(is_head) {
            return std::tuple(end, interval.upper());
          } else {
            return std::tuple(interval.lower(), start);
          }
        }();
        auto boundary_candlesticks_count = [&] {
          if(is_head) {
            return count_end_overlapping_candlesticks(candlesticks, end);
          } else {
            return count_start_overlapping_candlesticks(start, candlesticks);
          }
        }();
        auto new_limit = [&] {
          if(limit == SnapshotLimit::Unlimited()) {
            return limit;
          } else {
            return SnapshotLimit(limit.GetType(), limit.GetSize() +
              boundary_candlesticks_count);
          }
        }();
        return load(new_start, new_end, new_limit).then(
          [=, candlesticks = std::move(candlesticks)] (auto result) mutable {
            auto new_candlesticks = std::move(result.Get());
            auto [head, tail, offset] = [&] {
              if(is_head) {
                return std::tuple(&candlesticks, &new_candlesticks,
                  std::min(boundary_candlesticks_count,
                    new_candlesticks.size()));
              } else {
                return std::tuple(&new_candlesticks, &candlesticks,
                  std::min(boundary_candlesticks_count,
                    candlesticks.size()));
              }
            }();
            std::move(tail->begin() + offset, tail->end(),
              std::back_inserter(*head));
            return std::move(*head);
          });
      }
    });
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_queried_interval(
    const Interval& interval, const Interval& queried_interval,
    const SnapshotLimit& limit) {
  auto [future, promise] = make_future<void>();
  m_queried_intervals[queried_interval.lower()].m_futures.push_back(
    std::move(future));
  return promise.then([=] (auto result) {
    result.Get();
    return load(interval.lower(), interval.upper(), limit);
  });
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_new_interval(
    const Interval& interval,
    const boost::optional<Interval>& cached_interval,
    const boost::optional<Interval>& queried_interval,
    const SnapshotLimit& limit) {
  auto is_head = limit.GetType() == SnapshotLimit::Type::HEAD;
  auto start = interval.lower();
  auto end = interval.upper();
  if(is_head) {
    if(cached_interval && cached_interval->lower() < end) {
      end = cached_interval->lower();
    }
    if(queried_interval && queried_interval->lower() < end) {
      end = queried_interval->lower();
    }
  } else {
    if(cached_interval && start < cached_interval->upper()) {
      start = cached_interval->upper();
    }
    if(queried_interval && start < queried_interval->upper()) {
      start = queried_interval->upper();
    }
  }
  m_queried_intervals[start] = { end, {} };
  return m_model->load(start, end, limit).then([=] (auto result) {
    auto candlesticks = result.Get();
    m_cache.store(candlesticks);
    auto limit_size = static_cast<std::size_t>(limit.GetSize());
    if(candlesticks.size() < limit_size) {
      m_cached_intervals.insert(Interval::closed(start, end));
    } else {
      if(is_head && candlesticks.back().GetStart() > start) {
        m_cached_intervals.insert(Interval::right_open(start,
          candlesticks.back().GetStart()));
      } else if(!is_head && end > candlesticks.front().GetEnd()) {
        m_cached_intervals.insert(Interval::left_open(
          candlesticks.front().GetEnd(), end));
      }
    }
    auto futures = std::move(m_queried_intervals.at(start).m_futures);
    m_queried_intervals.erase(start);
    for(auto& future : futures) {
      future.resolve();
    }
    if(candlesticks.size() == limit_size) {
      return QtPromise(std::move(candlesticks));
    } else {
      return load(interval.lower(), interval.upper(), limit);
    }
  });
}

boost::optional<CachedChartModel::Interval>
    CachedChartModel::find_leftmost_cached_overlap(
    const Interval& interval) const {
  auto it = m_cached_intervals.lower_bound(interval);
  if(it == m_cached_intervals.end()) {
    return boost::none;
  } else {
    return *it;
  }
}

boost::optional<CachedChartModel::Interval>
    CachedChartModel::find_rightmost_cached_overlap(
    const Interval& interval) const {
  if(!m_cached_intervals.empty()) {
    auto it = m_cached_intervals.upper_bound(interval);
    if(it != m_cached_intervals.begin()) {
      --it;
      if(intersects(interval, *it)) {
        return *it;
      }
    }
  }
  return boost::none;
}

boost::optional<CachedChartModel::Interval>
    CachedChartModel::find_leftmost_queried_overlap(
    const Interval& interval) const {
  auto it = m_queried_intervals.lower_bound(interval.lower());
  if(it != m_queried_intervals.begin()) {
    auto prev = std::prev(it);
    if(prev->second.m_end > interval.lower()) {
      return Interval::closed(prev->first, prev->second.m_end);
    }
  }
  if(it != m_queried_intervals.end()) {
    if(it->first < interval.upper()) {
      return Interval::closed(it->first, it->second.m_end);
    }
  }
  return boost::none;
}

boost::optional<CachedChartModel::Interval>
    CachedChartModel::find_rightmost_queried_overlap(
    const Interval& interval) const {
  if(!m_queried_intervals.empty()) {
    auto it = m_queried_intervals.lower_bound(interval.upper());
    if(it != m_queried_intervals.begin()) {
      --it;
      if(it->second.m_end > interval.lower()) {
        return Interval::closed(it->first, it->second.m_end);
      }
    }
  }
  return boost::none;
}
