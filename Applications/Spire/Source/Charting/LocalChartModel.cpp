#include "Spire/Charting/LocalChartModel.hpp"
#include <iterator>

using namespace Beam::Queries;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;

LocalChartModel::LocalChartModel(ChartValue::Type x_axis_type,
    ChartValue::Type y_axis_type, std::vector<Candlestick> candlesticks)
    : m_x_axis_type(x_axis_type),
      m_y_axis_type(y_axis_type),
      m_candlesticks(std::move(candlesticks)) {
  std::sort(m_candlesticks.begin(), m_candlesticks.end(),
    [] (const auto& lhs, const auto& rhs) {
      return lhs.GetStart() < rhs.GetStart();
    });
}

ChartValue::Type LocalChartModel::get_x_axis_type() const {
  return m_x_axis_type;
}

ChartValue::Type LocalChartModel::get_y_axis_type() const {
  return m_y_axis_type;
}

QtPromise<std::vector<Candlestick>> LocalChartModel::load(ChartValue first,
    ChartValue last, const SnapshotLimit& limit) {
  return QtPromise([=, candlesticks = m_candlesticks] {
    auto start = std::lower_bound(candlesticks.begin(), candlesticks.end(),
      first, [] (const auto& lhs, const auto& rhs) {
        return lhs.GetEnd() < rhs;
      });
    auto end = std::lower_bound(start, candlesticks.end(),
      last, [] (const auto& lhs, const auto& rhs) {
        return lhs.GetStart() < rhs;
      });
    if(end != candlesticks.end()) {
      ++end;
    }
    if(std::distance(start, end) <= limit.GetSize()) {
      return std::vector<Candlestick>(start, end);
    } else if(limit.GetType() == SnapshotLimit::Type::HEAD) {
      return std::vector<Candlestick>(start, start + limit.GetSize());
    }
    return std::vector<Candlestick>(end - limit.GetSize(), end);
  });
}

connection LocalChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_candlestick_signal.connect(slot);
}
