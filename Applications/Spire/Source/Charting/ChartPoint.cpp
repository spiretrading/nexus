
#include "Spire/Charting/ChartPoint.hpp"

using namespace Spire;

ChartPoint::ChartPoint(ChartValue x, ChartValue y)
    : m_x(x),
      m_y(y) {}

bool ChartPoint::operator ==(const ChartPoint& rhs) const {
  return m_x == rhs.m_x && m_y == rhs.m_y;
}

bool ChartPoint::operator !=(const ChartPoint& rhs) const {
  return !(*this == rhs);
}

ChartPoint ChartPoint::operator +(const ChartPoint& rhs) const {
  return {m_x + rhs.m_x, m_y + rhs.m_y};
}

ChartPoint ChartPoint::operator -(const ChartPoint& rhs) const {
  return {m_x - rhs.m_x, m_y - rhs.m_y};
}
