#include "Spire/Charting/LocalChartModel.hpp"
#include <list>
#include <tuple>
#include <boost/signals2/connection.hpp>

using namespace Beam::Queries;
using namespace boost::signals2;
using namespace Spire;

namespace {
  auto are_overlapping(const Candlestick& candlestick,
      Scalar first, Scalar last) {
    return first <= candlestick.GetEnd() && last >= candlestick.GetStart();
  }
}

LocalChartModel::LocalChartModel(Scalar::Type x_axis_type,
    Scalar::Type y_axis_type, std::vector<Candlestick> candlesticks)
    : m_x_axis_type(x_axis_type),
      m_y_axis_type(y_axis_type) {
  store(candlesticks);
}

Scalar::Type LocalChartModel::get_x_axis_type() const {
  return m_x_axis_type;
}

Scalar::Type LocalChartModel::get_y_axis_type() const {
  return m_y_axis_type;
}

QtPromise<std::vector<Candlestick>> LocalChartModel::load(Scalar first,
    Scalar last, const SnapshotLimit& limit) {
  return QtPromise([=] {
    auto is_head = limit.GetType() == SnapshotLimit::Type::HEAD;
    auto iterators = std::list<std::tuple<Layer::const_iterator,
      Layer::const_iterator>>();
    for(auto& layer : m_layers) {
      if(is_head) {
        auto it = find_lower_bound(layer, first, last);
        if(it != layer.end()) {
          iterators.emplace_back(it, layer.end());
        }
      } else {
        auto it = find_upper_bound(layer, last);
        if(it != layer.begin()) {
          auto prev = std::prev(it);
          if(are_overlapping(prev->second, first, last)) {
            iterators.emplace_back(prev, layer.begin());
          }
        }
      }
    }
    auto result = std::vector<Candlestick>();
    for(auto i = 0; i < limit.GetSize() && !iterators.empty(); ++i) {
      static auto comparator = [] (auto& lhs, auto& rhs) {
        auto& left_candlestick = std::get<0>(lhs)->second;
        auto& right_candlestick = std::get<0>(rhs)->second;
        return left_candlestick.GetStart() < right_candlestick.GetStart() || (
          left_candlestick.GetStart() == right_candlestick.GetStart() &&
          left_candlestick.GetEnd() < right_candlestick.GetEnd());
      };
      auto it = [&] {
        if(is_head) {
          return std::min_element(iterators.begin(), iterators.end(),
            comparator);
        } else {
          return std::max_element(iterators.begin(), iterators.end(),
            comparator);
        }
      }();
      auto& candlestick_it = std::get<0>(*it);
      result.push_back(candlestick_it->second);
      if(is_head) {
        ++candlestick_it;
        if(candlestick_it == std::get<1>(*it) || !are_overlapping(
            candlestick_it->second, first, last)) {
          iterators.erase(it);
        }
      } else {
        if(candlestick_it == std::get<1>(*it) || !are_overlapping(
            std::prev(candlestick_it)->second, first, last)) {
          iterators.erase(it);
        } else {
          --candlestick_it;
        }
      }
    }
    if(!is_head) {
      std::reverse(result.begin(), result.end());
    }
    return result;
  });
}

void LocalChartModel::store(const std::vector<Candlestick>& candlesticks) {
  for(auto& candlestick : candlesticks) {
    auto inserted = false;
    for(auto& layer : m_layers) {
      if(test_insert(layer, candlestick)) {
        inserted = true;
        break;
      }
    }
    if(!inserted) {
      auto layer = Layer();
      layer[candlestick.GetStart()] = candlestick;
      m_layers.push_back(std::move(layer));
    }
  }
}

connection LocalChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_candlestick_signal.connect(slot);
}

bool LocalChartModel::test_insert(Layer& layer,
    const Candlestick& candlestick) {
  auto it = find_lower_bound(layer, candlestick.GetStart(),
    candlestick.GetEnd());
  if(it != layer.end() && it->second.GetEnd() == candlestick.GetStart()) {
    ++it;
  }
  if(it == layer.end() || it->first == candlestick.GetEnd() || (
      it->first == candlestick.GetStart() && it->second.GetEnd() ==
      candlestick.GetEnd())) {
    layer[candlestick.GetStart()] = candlestick;
    return true;
  }
  return false;
}

LocalChartModel::Layer::const_iterator LocalChartModel::find_lower_bound(
    const Layer& layer, Scalar first, Scalar last) {
  auto it = layer.lower_bound(first);
  if(it != layer.begin()) {
    auto prev = std::prev(it);
    if(prev->second.GetEnd() >= first) {
      return prev;
    }
  }
  if(it != layer.end() && it->second.GetEnd() >= first) {
    return it;
  }
  return layer.end();
}

LocalChartModel::Layer::const_iterator LocalChartModel::find_upper_bound(
    const Layer& layer, Scalar last) {
  return layer.upper_bound(last);
}
