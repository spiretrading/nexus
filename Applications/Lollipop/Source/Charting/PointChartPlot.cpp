#include "Spire/Charting/PointChartPlot.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

PointChartPlot::PointChartPlot() {}

PointChartPlot::PointChartPlot(ChartValue x, ChartValue y)
    : m_x(x),
      m_y(y) {}

PointChartPlot::~PointChartPlot() {}

ChartValue PointChartPlot::GetX() const {
  return m_x;
}

void PointChartPlot::SetX(ChartValue value) {
  if(value == m_x) {
    return;
  }
  m_x = value;
  m_updateSignal();
}

ChartValue PointChartPlot::GetY() const {
  return m_y;
}

void PointChartPlot::SetY(ChartValue value) {
  if(value == m_y) {
    return;
  }
  m_y = value;
  m_updateSignal();
}

connection PointChartPlot::ConnectUpdateSignal(
    const UpdateSignal::slot_function_type& slot) const {
  return m_updateSignal.connect(slot);
}
