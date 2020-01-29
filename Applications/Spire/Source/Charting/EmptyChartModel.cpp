#include "Spire/Charting/EmptyChartModel.hpp"

using namespace Beam::Queries;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;

EmptyChartModel::EmptyChartModel(Scalar::Type x_axis_type,
  Scalar::Type y_axis_type)
  : m_x_axis_type(x_axis_type),
    m_y_axis_type(y_axis_type) {}

Scalar::Type EmptyChartModel::get_x_axis_type() const {
  return m_x_axis_type;
}

Scalar::Type EmptyChartModel::get_y_axis_type() const {
  return m_y_axis_type;
}

QtPromise<std::vector<Candlestick>> EmptyChartModel::load(Scalar first,
    Scalar last, const SnapshotLimit& limit) {
  return QtPromise([] {
    return std::vector<Candlestick>();
  });
}

connection EmptyChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_candlestick_signal.connect(slot);
}
