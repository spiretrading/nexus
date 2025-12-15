#include "Spire/Dashboard/DashboardRowRenderer.hpp"
#include <QPainter>
#include "Spire/Dashboard/DashboardRow.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

DashboardRowRenderer::DashboardRowRenderer(Ref<const DashboardRow> row,
    const DashboardCellRendererBuilder& cellRendererBuilder)
    : m_row{row.get()},
      m_cellRendererBuilder{cellRendererBuilder},
      m_defaultCellWidth{75} {
  for(auto i = 0; i < m_row->GetSize(); ++i) {
    OnCellAddedSignal(m_row->GetCell(i));
  }
  m_cellAddedConnection = m_row->ConnectCellAddedSignal(std::bind(
    &DashboardRowRenderer::OnCellAddedSignal, this, std::placeholders::_1));
}

const DashboardRow& DashboardRowRenderer::GetRow() const {
  return *m_row;
}

const DashboardCellRenderer& DashboardRowRenderer::GetCellRenderer(
    int index) const {
  return *m_cells[index]->m_renderer;
}

DashboardCellRenderer& DashboardRowRenderer::GetCellRenderer(int index) {
  return *m_cells[index]->m_renderer;
}

int DashboardRowRenderer::GetCellWidth(int index) const {
  return m_cells[index]->m_width;
}

void DashboardRowRenderer::SetCellWidth(int index, int width) {
  if(m_cells[index]->m_width == width) {
    return;
  }
  m_cells[index]->m_width = width;
  m_drawSignal();
}

void DashboardRowRenderer::Move(int sourceIndex, int destinationIndex) {
  if(sourceIndex == destinationIndex) {
    return;
  }
  auto cell = std::move(m_cells[sourceIndex]);
  m_cells.erase(m_cells.begin() + sourceIndex);
  m_cells.insert(m_cells.begin() + destinationIndex, std::move(cell));
  m_drawSignal();
}

void DashboardRowRenderer::Draw(QPaintDevice& device, const QRect& region) {
  auto leftmostPoint = region.left();
  for(auto& cell : m_cells) {
    QRect cellRegion{leftmostPoint, region.top(), cell->m_width,
      region.height()};
    cell->m_renderer->Draw(device, cellRegion);
    leftmostPoint += cell->m_width;
  }
}

connection DashboardRowRenderer::ConnectDrawSignal(
    const DrawSignal::slot_function_type& slot) const {
  return m_drawSignal.connect(slot);
}

void DashboardRowRenderer::OnCellAddedSignal(const DashboardCell& cell) {
  auto cellEntry = std::make_unique<CellEntry>();
  cellEntry->m_cell = &cell;
  cellEntry->m_width = m_defaultCellWidth;
  cellEntry->m_renderer = m_cellRendererBuilder(*cellEntry->m_cell);
  cellEntry->m_drawConnection = cellEntry->m_renderer->ConnectDrawSignal(
    std::bind(&DashboardRowRenderer::OnCellDrawSignal, this));
  m_cells.push_back(std::move(cellEntry));
  m_drawSignal();
}

void DashboardRowRenderer::OnCellDrawSignal() {
  m_drawSignal();
}
