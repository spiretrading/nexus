#include "Spire/Charting/ChartPoint.hpp"

using namespace Spire;

ChartPoint::ChartPoint()
    : m_x(),
      m_y() {}

ChartPoint::ChartPoint(ChartValue x, ChartValue y)
    : m_x(x),
      m_y(y) {}
