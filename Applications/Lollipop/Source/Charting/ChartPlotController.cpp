#include "Spire/Charting/ChartPlotController.hpp"
#include "Spire/Charting/CandlestickChartPlot.hpp"
#include "Spire/Charting/ChartPlotSeries.hpp"
#include "Spire/Charting/PointChartPlot.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  static const int UPDATE_INTERVAL = 1000;

  ChartValue GetTopViewPoint(const ChartPlot& plot) {
    if(auto candlestickPlot =
        dynamic_cast<const CandlestickChartPlot*>(&plot)) {
      return candlestickPlot->GetValue().GetHigh();
    }
    if(auto pointPlot = dynamic_cast<const PointChartPlot*>(&plot)) {
      return pointPlot->GetY();
    }
    return ChartValue();
  }

  ChartValue GetBottomViewPoint(const ChartPlot& plot) {
    if(auto candlestickPlot =
        dynamic_cast<const CandlestickChartPlot*>(&plot)) {
      return candlestickPlot->GetValue().GetLow();
    }
    if(auto pointPlot = dynamic_cast<const PointChartPlot*>(&plot)) {
      return pointPlot->GetY();
    }
    return ChartValue();
  }

  ChartValue GetLeftViewPoint(const ChartPlot& plot) {
    if(auto candlestickPlot =
        dynamic_cast<const CandlestickChartPlot*>(&plot)) {
      return candlestickPlot->GetValue().GetStart();
    }
    if(auto pointPlot = dynamic_cast<const PointChartPlot*>(&plot)) {
      return pointPlot->GetX();
    }
    return ChartValue();
  }

  ChartValue GetRightViewPoint(const ChartPlot& plot) {
    if(auto candlestickPlot =
        dynamic_cast<const CandlestickChartPlot*>(&plot)) {
      return candlestickPlot->GetValue().GetEnd();
    }
    if(auto pointPlot = dynamic_cast<const PointChartPlot*>(&plot)) {
      return pointPlot->GetX();
    }
    return ChartValue();
  }
}

bool ChartPlotController::ChartPlotComparator::operator ()(
      const std::shared_ptr<ChartPlot>& a,
      const std::shared_ptr<ChartPlot>& b) const {
  return GetLeftViewPoint(*a) < GetLeftViewPoint(*b);
}

ChartPlotController::RangeEntry::RangeEntry(ChartValue min, ChartValue max)
    : m_min(min),
      m_max(max) {}

ChartPlotController::ChartPlotController()
    : m_view(nullptr),
      m_isAutoScaleEnabled(false),
      m_isLockGridEnabled(false),
      m_lastValueIndex(0) {
  std::function<void ()> timeoutSlot = std::bind(
    &ChartPlotController::OnUpdateTimer, this);
  QObject::connect(&m_updateTimer, &QTimer::timeout, timeoutSlot);
  m_updateTimer.start(UPDATE_INTERVAL);
}

bool ChartPlotController::IsAutoScaleEnabled() const {
  return m_isAutoScaleEnabled;
}

void ChartPlotController::SetAutoScale(bool autoScale) {
  if(autoScale == m_isAutoScaleEnabled) {
    return;
  }
  m_isAutoScaleEnabled = autoScale;
  UpdateAutoScale();
}

bool ChartPlotController::IsLockGridEnabled() const {
  return m_isLockGridEnabled;
}

void ChartPlotController::SetLockGrid(bool lockGrid) {
  if(lockGrid == m_isLockGridEnabled) {
    return;
  }
  m_isLockGridEnabled = lockGrid;
  if(m_isLockGridEnabled) {
    m_lastValue = std::nullopt;
    ++m_lastValueIndex;
  }
  UpdateLockGrid();
}

void ChartPlotController::SetView(Ref<ChartPlotView> view) {
  m_view = view.Get();
  m_xAxisParametersConnection = m_view->ConnectXAxisParametersChangedSignal(
    std::bind(&ChartPlotController::OnXAxisParametersChanged, this,
    std::placeholders::_1));
  OnXAxisParametersChanged(m_view->GetXAxisParameters());
}

void ChartPlotController::Add(const std::shared_ptr<ChartPlotSeries>& series) {
  m_series.push_back(series);
  m_seriesConnections.AddConnection(series.get(),
    series->ConnectChartPointAddedSignal(std::bind(
    &ChartPlotController::OnChartPlotAdded, this, std::placeholders::_1)));
  if(m_view != nullptr) {
    QueryRange(series, m_view->GetXAxisParameters().m_min,
      m_view->GetXAxisParameters().m_max);
  }
  UpdateLockGrid();
}

void ChartPlotController::Clear() {
  m_lastValue = std::nullopt;
  ++m_lastValueIndex;
  m_view->Clear();
  m_series.clear();
  m_seriesConnections.DisconnectAll();
  m_ranges.clear();
  m_plots.clear();
  m_plotConnections.DisconnectAll();
}

boost::optional<ChartValue> ChartPlotController::LoadLastValue() {
  if(m_lastValue.has_value()) {
    return *m_lastValue;
  }
  m_taskQueue.Push(
    [=, series = m_series, lastValueIndex = m_lastValueIndex] {
      auto lastValue = ChartValue(std::numeric_limits<Quantity>::min());
      for(auto& s : series) {
        try {
          auto seriesLastValue = s->LoadLastCurrentDomain();
          lastValue = std::max(lastValue, seriesLastValue);
        } catch(const std::exception&) {}
      }
      if(lastValue != ChartValue(std::numeric_limits<Quantity>().min())) {
        m_lastValuesLoaded.PushBack(std::tuple(lastValueIndex, lastValue));
      }
    });
  return none;
}

void ChartPlotController::UpdateAutoScale() {
  if(!m_isAutoScaleEnabled) {
    return;
  }
  auto leftPoint = std::make_shared<PointChartPlot>(m_xAxisParameters.m_min,
    ChartValue());
  auto leftIterator = std::lower_bound(m_plots.begin(), m_plots.end(),
    leftPoint, ChartPlotComparator());
  if(leftIterator == m_plots.end()) {
    return;
  }
  auto rightPoint = std::make_shared<PointChartPlot>(m_xAxisParameters.m_max,
    ChartValue());
  auto rightIterator = std::lower_bound(m_plots.begin(), m_plots.end(),
    rightPoint, ChartPlotComparator());
  auto min = ChartValue{std::numeric_limits<Quantity>::max()};
  auto max = ChartValue{std::numeric_limits<Quantity>::min()};
  for(auto i = leftIterator; i != rightIterator; ++i) {
    min = std::min(min, GetBottomViewPoint(**i));
    max = std::max(max, GetTopViewPoint(**i));
  }
  if(min == max) {
    min -= ChartValue{Nexus::Money::CENT};
    max += ChartValue{Nexus::Money::CENT};
  }
  auto yAxisParameters = m_view->GetYAxisParameters();
  yAxisParameters.m_min = min;
  yAxisParameters.m_max = max;
  if(yAxisParameters.m_min == m_view->GetYAxisParameters().m_min &&
      yAxisParameters.m_max == m_view->GetYAxisParameters().m_max) {
    return;
  }
  m_view->SetYAxisParameters(yAxisParameters);
}

void ChartPlotController::UpdateLockGrid() {
  if(!m_isLockGridEnabled) {
    return;
  }
  auto lastValue = LoadLastValue();
  if(!lastValue.is_initialized()) {
    return;
  }
  auto xAxisParameters = m_view->GetXAxisParameters();
  auto range = xAxisParameters.m_max - xAxisParameters.m_min;
  xAxisParameters.m_max = *lastValue + (5 * range) / 100;
  xAxisParameters.m_min = xAxisParameters.m_max - range;
  if(xAxisParameters.m_min == m_view->GetXAxisParameters().m_min &&
      xAxisParameters.m_max == m_view->GetXAxisParameters().m_max) {
    return;
  }
  m_view->SetXAxisParameters(xAxisParameters);
}

void ChartPlotController::QueryRange(
    const std::shared_ptr<ChartPlotSeries>& series, ChartValue min,
    ChartValue max) {
  auto& rangeEntries = m_ranges[series];
  auto rangeEntryIterator = rangeEntries.begin();
  while(rangeEntryIterator != rangeEntries.end()) {
    auto& rangeEntry = *rangeEntryIterator;
    if(max <= rangeEntry.m_min) {
      break;
    } else if(min >= rangeEntry.m_min && max <= rangeEntry.m_max) {
      return;
    } else if(max > rangeEntry.m_min && max <= rangeEntry.m_max) {
      series->Query(min, rangeEntry.m_min);
      rangeEntry.m_min = min;
      return;
    } else if(min >= rangeEntry.m_min && min <= rangeEntry.m_max) {
      series->Query(rangeEntry.m_max, max);
      rangeEntry.m_max = max;
      return;
    }
    ++rangeEntryIterator;
  }
  rangeEntries.insert(rangeEntryIterator, RangeEntry(min, max));
  series->Query(min, max);
}

void ChartPlotController::OnChartPlotAdded(
    const std::shared_ptr<ChartPlot>& plot) {
  if(m_view != nullptr) {
    m_view->Plot(plot);
  }
  m_plots.insert(plot);
  m_plotConnections.AddConnection(plot.get(), plot->ConnectUpdateSignal(
    std::bind(&ChartPlotController::OnPlotUpdated, this,
    std::weak_ptr<ChartPlot>(plot))));
  UpdateAutoScale();
  if(m_lastValue.has_value()) {
    *m_lastValue = std::max(*m_lastValue, GetRightViewPoint(*plot));
  } else {
    m_lastValue.emplace(GetRightViewPoint(*plot));
  }
  UpdateLockGrid();
}

void ChartPlotController::OnPlotUpdated(std::weak_ptr<ChartPlot> weakPlot) {
  auto plot = weakPlot.lock();
  if(plot == nullptr) {
    return;
  }
  if(GetLeftViewPoint(*plot) >= m_xAxisParameters.m_min &&
      GetLeftViewPoint(*plot) <= m_xAxisParameters.m_max ||
      GetRightViewPoint(*plot) >= m_xAxisParameters.m_min &&
      GetRightViewPoint(*plot) <= m_xAxisParameters.m_max) {
    UpdateAutoScale();
  }
  if(m_lastValue.has_value()) {
    *m_lastValue = std::max(*m_lastValue, GetRightViewPoint(*plot));
  } else {
    m_lastValue.emplace(GetRightViewPoint(*plot));
  }
}

void ChartPlotController::OnXAxisParametersChanged(
    const ChartPlotView::AxisParameters& parameters) {
  if(m_xAxisParameters.m_min == parameters.m_min &&
      m_xAxisParameters.m_max == parameters.m_max) {
    m_xAxisParameters = parameters;
    return;
  }
  m_xAxisParameters = parameters;
  for(const std::shared_ptr<ChartPlotSeries>& series : m_series) {
    QueryRange(series, parameters.m_min, parameters.m_max);
  }
  UpdateAutoScale();
  UpdateLockGrid();
}

void ChartPlotController::OnUpdateTimer() {
  vector<std::tuple<uint64_t, ChartValue>> lastValuesLoaded;
  m_lastValuesLoaded.Swap(lastValuesLoaded);
  bool update = false;
  for(const std::tuple<uint64_t, ChartValue>& lastValueLoaded :
      lastValuesLoaded) {
    if(std::get<0>(lastValueLoaded) == m_lastValueIndex) {
      if(m_lastValue.has_value()) {
        *m_lastValue = std::max(*m_lastValue, std::get<1>(lastValueLoaded));
      } else {
        m_lastValue.emplace(std::get<1>(lastValueLoaded));
      }
      update = true;
    }
  }
  if(update) {
    UpdateLockGrid();
  }
}
