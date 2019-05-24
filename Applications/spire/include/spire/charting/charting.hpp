#ifndef SPIRE_CHARTING_HPP
#define SPIRE_CHARTING_HPP
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"
#include "spire/spire/spire.hpp"

namespace Spire {
  class CachedChartModel;
  class ChartModel;
  struct ChartPoint;
  class ChartValue;
  class ChartView;
  class ChartingTechnicalsPanel;
  class ChartingWindow;
  class TrendLineEditor;
  class TrendLineModel;
  using Candlestick =
    Nexus::TechnicalAnalysis::Candlestick<ChartValue, ChartValue>;
}

#endif
