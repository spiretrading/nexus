#include "Spire/Charting/LocalChartModel.hpp"

using namespace Beam::Queries;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;

LocalChartModel::LocalChartModel(Scalar::Type x_axis_type,
    Scalar::Type y_axis_type, std::vector<Candlestick> candlesticks)
    : m_x_axis_type(x_axis_type),
      m_y_axis_type(y_axis_type),
      m_candlesticks(std::move(candlesticks)) {
  std::sort(m_candlesticks.begin(), m_candlesticks.end(),
    [] (const auto& lhs, const auto& rhs) {
      return lhs.GetStart() < rhs.GetStart();
    });
}

Scalar::Type LocalChartModel::get_x_axis_type() const {
  return m_x_axis_type;
}

Scalar::Type LocalChartModel::get_y_axis_type() const {
  return m_y_axis_type;
}

QtPromise<std::vector<Candlestick>> LocalChartModel::load(Scalar first,
    Scalar last, const SnapshotLimit& limit) {
  return QtPromise([=, candlesticks = m_candlesticks] {
    auto start = std::lower_bound(candlesticks.begin(), candlesticks.end(),
      first, [] (const auto& lhs, const auto& rhs) {
        return lhs.GetEnd() < rhs;
      });
    auto end = std::upper_bound(start, candlesticks.end(),
      last, [] (const auto& lhs, const auto& rhs) {
        return lhs < rhs.GetStart();
      });
    if(std::distance(start, end) <= limit.GetSize()) {
      return std::vector<Candlestick>(start, end);
    } else if(limit.GetType() == SnapshotLimit::Type::HEAD) {
      return std::vector<Candlestick>(start, start + limit.GetSize());
    }
    return std::vector<Candlestick>(end - limit.GetSize(), end);
  });
}

void LocalChartModel::store(const std::vector<Candlestick>& candlesticks) {
  if(candlesticks.empty()) {
    return;
  }
  m_candlesticks.insert(m_candlesticks.end(), candlesticks.begin(),
    candlesticks.end());
  std::inplace_merge(m_candlesticks.begin(), m_candlesticks.begin() +
    (m_candlesticks.size() - candlesticks.size()), m_candlesticks.end(),
    [] (const auto& first, const auto& second) {
      return first.GetStart() < second.GetStart();
    });
}

connection LocalChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_candlestick_signal.connect(slot);
}
