#ifndef SPIRE_CHARTING_HPP
#define SPIRE_CHARTING_HPP
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  class CachedChartModel;
  class ChartModel;
  struct ChartPoint;
  class ChartValue;
  class ChartView;
  class ChartingTechnicalsPanel;
  class ChartingWindow;
  class ColorPicker;
  class DropDownColorPicker;
  class StyleDropDownMenuItem;
  class StyleDropDownMenuList;
  class TrendLineEditor;
  class TrendLineModel;
  class TrendLineStyleDropdownMenu;
  using Candlestick =
    Nexus::TechnicalAnalysis::Candlestick<ChartValue, ChartValue>;
}

#endif
