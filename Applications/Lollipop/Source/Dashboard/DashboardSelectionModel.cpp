#include "Spire/Dashboard/DashboardSelectionModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

boost::optional<int> DashboardSelectionModel::GetActiveRow() const {
  return m_activeRow;
}

const vector<int>& DashboardSelectionModel::GetSelectedRows() const {
  return m_selectedRows;
}

bool DashboardSelectionModel::IsRowSelected(int row) const {
  return find(m_selectedRows.begin(), m_selectedRows.end(), row) !=
    m_selectedRows.end();
}

void DashboardSelectionModel::AddRow(int index) {
  auto insertIterator = lower_bound(m_selectedRows.begin(),
    m_selectedRows.end(), index);
  if(insertIterator != m_selectedRows.end() && *insertIterator == index) {
    return;
  }
  m_selectedRows.insert(insertIterator, index);
  m_selectedRowsUpdatedSignal();
}

void DashboardSelectionModel::RemoveRow(int index) {
  auto rowIterator = lower_bound(m_selectedRows.begin(), m_selectedRows.end(),
    index);
  if(rowIterator == m_selectedRows.end() || *rowIterator != index) {
    return;
  }
  m_selectedRows.erase(rowIterator);
  m_selectedRowsUpdatedSignal();
}

void DashboardSelectionModel::SetActiveRow(const boost::optional<int>& index) {
  if(index == m_activeRow) {
    return;
  }
  m_activeRow = index;
  m_activeRowUpdatedSignal(m_activeRow);
}

void DashboardSelectionModel::Reset() {
  if(!m_selectedRows.empty()) {
    m_selectedRows.clear();
    m_selectedRowsUpdatedSignal();
  }
  if(m_activeRow != none) {
    m_activeRow = none;
    m_activeRowUpdatedSignal(m_activeRow);
  }
}

connection DashboardSelectionModel::ConnectSelectedRowsUpdatedSignal(
    const SelectedRowsUpdatedSignal::slot_function_type& slot) const {
  return m_selectedRowsUpdatedSignal.connect(slot);
}

connection DashboardSelectionModel::ConnectActiveRowUpdatedSignal(
    const ActiveRowUpdatedSignal::slot_function_type& slot) const {
  return m_activeRowUpdatedSignal.connect(slot);
}
