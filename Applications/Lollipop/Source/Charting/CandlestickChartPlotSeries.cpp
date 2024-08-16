#include "Spire/Charting/CandlestickChartPlotSeries.hpp"
#include "Spire/Charting/CandlestickChartPlot.hpp"
#include "Spire/Charting/ChartPoint.hpp"
#include "Spire/Charting/ChartPointSeries.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

CandlestickChartPlotSeries::CandlestickChartPlotSeries(ChartValue domain,
    unique_ptr<ChartPointSeries> chartPointSeries)
    : m_domain(domain),
      m_chartPointSeries(std::move(chartPointSeries)) {
  m_seriesConnection = m_chartPointSeries->ConnectChartPointAddedSignal(
    std::bind(&CandlestickChartPlotSeries::OnChartPointAdded, this,
    std::placeholders::_1));
}

CandlestickChartPlotSeries::~CandlestickChartPlotSeries() {}

void CandlestickChartPlotSeries::Query(ChartValue start, ChartValue end) {
  m_chartPointSeries->Query(start, end);
}

connection CandlestickChartPlotSeries::ConnectChartPointAddedSignal(
    const ChartPlotAddedSignal::slot_function_type& slot) const {
  return m_chartPlotAddedSignal.connect(slot);
}

void CandlestickChartPlotSeries::OnChartPointAdded(
    const ChartPoint& chartPoint) {
  auto plotIterator = std::find_if(m_plots.begin(), m_plots.end(),
    [&] (const std::shared_ptr<CandlestickChartPlot>& plot) -> bool {
      return chartPoint.m_x >= plot->GetValue().GetStart() &&
        chartPoint.m_x <= plot->GetValue().GetEnd();
    });
  bool signalPlot = false;
  std::shared_ptr<CandlestickChartPlot> plot;
  if(plotIterator == m_plots.end()) {
    ChartValue start = chartPoint.m_x - (chartPoint.m_x % m_domain);
    ChartValue end = start + m_domain;
    plot = std::make_shared<CandlestickChartPlot>(start, end);
    m_plots.push_back(plot);
    signalPlot = true;
  } else {
    plot = *plotIterator;
  }
  plot->Update(chartPoint.m_y);
  if(signalPlot) {
    m_chartPlotAddedSignal(plot);
  }
}
