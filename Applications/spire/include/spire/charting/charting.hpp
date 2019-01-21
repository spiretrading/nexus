#ifndef SPIRE_CHARTING_HPP
#define SPIRE_CHARTING_HPP
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"
#include "spire/spire/spire.hpp"

namespace Spire {
  struct ChartPoint;
  class ChartValue;
  class ChartView;
  class ChartingWindow;
  using Candlestick =
    Nexus::TechnicalAnalysis::Candlestick<ChartValue, ChartValue>;
}

#endif
