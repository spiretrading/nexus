#include "Spire/Dashboard/DashboardCellRenderer.hpp"

using namespace Beam;
using namespace Spire;

const DashboardCell& DashboardCellRenderer::GetCell() const {
  return *m_cell;
}

DashboardCellRenderer::DashboardCellRenderer(RefType<const DashboardCell> cell)
    : m_cell{cell.Get()} {}
