#include "Spire/Charting/TimeAndSalesChartPointSeries.hpp"
#include <Beam/Queues/Publisher.hpp>
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Spire/Charting/ChartPoint.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

TimeAndSalesChartPointSeries::TimeAndSalesChartPointSeries(
    const Publisher<TimeAndSale>& publisher)
    : m_publisher(&publisher) {
}

TimeAndSalesChartPointSeries::~TimeAndSalesChartPointSeries() {}

void TimeAndSalesChartPointSeries::Query(ChartValue start, ChartValue end) {
  if(m_publisher == nullptr) {
    return;
  }
  m_publisher->monitor(m_taskQueue.get_slot<TimeAndSale>(std::bind(
    &TimeAndSalesChartPointSeries::OnTimeAndSale, this,
    std::placeholders::_1)));
  m_publisher = nullptr;
}

connection TimeAndSalesChartPointSeries::ConnectChartPointAddedSignal(
    const ChartPointAddedSignal::slot_function_type& slot) const {
  return m_chartPointAddedSignal.connect(slot);
}

void TimeAndSalesChartPointSeries::OnTimeAndSale(
    const TimeAndSale& timeAndSale) {
  ChartPoint point(ChartValue(timeAndSale.m_timestamp),
    ChartValue(timeAndSale.m_price));
  m_chartPointAddedSignal(point);
}
