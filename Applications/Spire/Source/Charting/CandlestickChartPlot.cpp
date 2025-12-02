#include "Spire/Charting/CandlestickChartPlot.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

CandlestickChartPlot::CandlestickChartPlot(ChartValue start, ChartValue end)
    : m_value(start, end) {}

CandlestickChartPlot::CandlestickChartPlot(const CandlestickValue& value)
    : m_value(value) {}

CandlestickChartPlot::~CandlestickChartPlot() {}

const CandlestickChartPlot::CandlestickValue&
    CandlestickChartPlot::GetValue() const {
  return m_value;
}

void CandlestickChartPlot::Update(ChartValue value) {
  m_value.update(value);
  m_updateSignal();
}

connection CandlestickChartPlot::ConnectUpdateSignal(
    const UpdateSignal::slot_function_type& slot) const {
  return m_updateSignal.connect(slot);
}
