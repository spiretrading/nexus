#include "Spire/Charting/ChartPlotViewWindowSettings.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

ChartPlotViewWindowSettings::ChartPlotViewWindowSettings(
    const ChartPlotView& window)
    : m_properties(window.GetProperties()),
      m_xAxisParameters(window.GetXAxisParameters()),
      m_yAxisParameters(window.GetYAxisParameters()),
      m_geometry(window.saveGeometry()) {}

ChartPlotViewWindowSettings::~ChartPlotViewWindowSettings() {}

QWidget* ChartPlotViewWindowSettings::Reopen(
    Ref<UserProfile> userProfile) const {
  ChartPlotView* view = new ChartPlotView();
  Apply(Ref(userProfile), Store(*view));
  return view;
}

void ChartPlotViewWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  ChartPlotView& view = dynamic_cast<ChartPlotView&>(*widget);
  view.restoreGeometry(m_geometry);
  view.Initialize(Ref(userProfile), m_properties);
  view.SetXAxisParameters(m_xAxisParameters);
  view.SetYAxisParameters(m_yAxisParameters);
}

ChartPlotViewWindowSettings::ChartPlotViewWindowSettings() {}
