#include "spire/charting/local_chart_model.hpp"
#include "spire/charting/chart_value.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

LocalChartModel::LocalChartModel(std::vector<Candlestick> candlesticks)
    : m_candlesticks(std::move(candlesticks)) {
  std::sort(m_candlesticks.begin(), m_candlesticks.end(),
    [] (const auto& lhs, const auto& rhs) {
      return lhs.GetStart() < rhs.GetStart();
    });
}

QtPromise<std::vector<Candlestick>> LocalChartModel::load(ChartValue first,
    ChartValue last) {
  return make_qt_promise([=, candlesticks = m_candlesticks] {
    auto start = std::lower_bound(candlesticks.begin(), candlesticks.end(),
      first, [] (const auto& lhs, const auto& rhs) {
        return lhs.GetStart() < rhs;
      });
    auto end = std::lower_bound(start, candlesticks.end(),
      last, [] (const auto& lhs, const auto& rhs) {
        return lhs.GetStart() < rhs;
      });
    return std::vector<Candlestick>(start, end);
  });
}

connection LocalChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_candlestick_signal.connect(slot);
}
