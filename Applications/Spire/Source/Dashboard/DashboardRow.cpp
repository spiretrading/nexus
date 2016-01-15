#include "Spire/Dashboard/DashboardRow.hpp"
#include <cassert>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

DashboardRow::DashboardRow(std::unique_ptr<DashboardCell> index) {
  m_cells.push_back(std::move(index));
}

int DashboardRow::GetSize() const {
  return m_cells.size();
}

const DashboardCell& DashboardRow::GetIndex() const {
  return GetCell(0);
}

const DashboardCell& DashboardRow::GetCell(int column) const {
  assert(column >= 0 && column < static_cast<int>(m_cells.size()));
  return *m_cells[column];
}

void DashboardRow::Add(std::unique_ptr<DashboardCell> cell) {
  m_cells.push_back(std::move(cell));
  m_cellAddedSignal(*m_cells.back());
}

void DashboardRow::Remove(int column) {
  assert(column > 0 && column < static_cast<int>(m_cells.size()));
  m_cells.erase(m_cells.begin() + column);
}

connection DashboardRow::ConnectCellAddedSignal(
    const CellAddedSignal::slot_function_type& slot) const {
  return m_cellAddedSignal.connect(slot);
}
