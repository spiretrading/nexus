#include "Spire/Charting/LocalChartModel.hpp"
#include <boost/range/adaptor/reversed.hpp>

using namespace Beam::Queries;
using namespace boost;
using namespace boost::adaptors;
using namespace boost::signals2;
using namespace Spire;

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
    auto limit_size = static_cast<std::size_t>(limit.GetSize());
    auto result = std::vector<Candlestick>();
    if(limit.GetType() == SnapshotLimit::Type::HEAD) {
      for(auto& [_, candlestick] : m_candlesticks) {
        if(result.size() == limit_size || candlestick.GetStart() > last) {
          break;
        }
        if(candlestick.GetEnd() >= first) {
          result.push_back(candlestick);
        }
      }
    } else {
      for(auto& [_, candlestick] : reverse(m_candlesticks)) {
        if(result.size() == limit_size) {
          break;
        }
        if(candlestick.GetStart() <= last && candlestick.GetEnd() >= first) {
          result.push_back(candlestick);
        }
      }
      std::reverse(result.begin(), result.end());
    }
    return result;
  });
}

void LocalChartModel::store(const std::vector<Candlestick>& candlesticks) {
  for(auto& candlestick : candlesticks) {
    m_candlesticks[{candlestick.GetStart(), candlestick.GetEnd()}] =
      candlestick;
  }
}

connection LocalChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_candlestick_signal.connect(slot);
}

bool LocalChartModel::CandlestickDomain::operator <(
    const CandlestickDomain& other) const {
  return m_start < other.m_start || (m_start == other.m_start &&
    m_end < other.m_end);
}
