#include "Spire/Charting/EmptyChartModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

EmptyChartModel::EmptyChartModel(ChartValue::Type x_axis_type,
    ChartValue::Type y_axis_type)
    : m_x_axis_type(x_axis_type),
      m_y_axis_type(y_axis_type) {}

ChartValue::Type EmptyChartModel::get_x_axis_type() const {
  return m_x_axis_type;
}

ChartValue::Type EmptyChartModel::get_y_axis_type() const {
  return m_y_axis_type;
}

QtPromise<std::vector<Candlestick>> EmptyChartModel::load(ChartValue first,
    ChartValue last) {
  return QtPromise([] {
    return std::vector<Candlestick>();
  });
}

connection EmptyChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_candlestick_signal.connect(slot);
}
