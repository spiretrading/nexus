#include "Spire/Dashboard/DashboardModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

DashboardModel::DashboardModel(vector<string> columnNames)
    : m_columnNames(std::move(columnNames)) {}

int DashboardModel::GetRowCount() const {
  return m_rows.size();
}

int DashboardModel::GetColumnCount() const {
  return m_columnNames.size();
}

const string& DashboardModel::GetColumnName(int index) const {
  return m_columnNames[index];
}

void DashboardModel::SetColumnName(int index, string name) {
  m_columnNames[index] = std::move(name);
}

const DashboardRow& DashboardModel::GetRow(int index) const {
  return *m_rows[index];
}

void DashboardModel::Add(unique_ptr<DashboardRow> row) {
  assert(row->GetSize() == GetColumnCount());
  m_rows.push_back(std::move(row));
  m_rowAddedSignal(*m_rows.back());
}

void DashboardModel::Remove(const DashboardRow& row) {
  for(auto i = m_rows.begin(); i != m_rows.end(); ++i) {
    if(i->get() == &row) {
      auto selfRow = std::move(*i);
      m_rows.erase(i);
      m_rowRemovedSignal(*selfRow);
      break;
    }
  }
}

connection DashboardModel::ConnectRowAddedSignal(
    const RowAddedSignal::slot_function_type& slot) const {
  return m_rowAddedSignal.connect(slot);
}

connection DashboardModel::ConnectRowRemovedSignal(
    const RowRemovedSignal::slot_function_type& slot) const {
  return m_rowRemovedSignal.connect(slot);
}
