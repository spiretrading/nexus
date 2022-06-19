#include "Spire/Blotter/PortfolioToPositionsTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

PortfolioToPositionsTableModel::PortfolioToPositionsTableModel(
  std::shared_ptr<PortfolioModel> portfolio)
  : m_portfolio(std::move(portfolio)),
    m_connection(m_portfolio->connect_update_signal(
      std::bind_front(&PortfolioToPositionsTableModel::on_update, this))) {}

int PortfolioToPositionsTableModel::get_row_size() const {
  return m_table.get_row_size();
}

int PortfolioToPositionsTableModel::get_column_size() const {
  return COLUMN_SIZE;
}

AnyRef PortfolioToPositionsTableModel::at(int row, int column) const {
  return m_table.at(row, column);
}

connection PortfolioToPositionsTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_table.connect_operation_signal(slot);
}

void PortfolioToPositionsTableModel::on_update(
    const PortfolioModel::Portfolio::UpdateEntry& update) {
  auto& security = update.m_securityInventory.m_position.m_key.m_index;
  auto index = m_indexes.find(security);
  if(index == m_indexes.end()) {
    m_indexes.insert(std::pair(security, m_table.get_row_size())).first;
    auto row = std::vector<std::any>();
    row.push_back(security);
    row.push_back(Abs(update.m_securityInventory.m_position.m_quantity));
    row.push_back(GetSide(update.m_securityInventory.m_position.m_quantity));
    row.push_back(GetAveragePrice(update.m_securityInventory.m_position));
    row.push_back(update.m_unrealizedSecurity);
    row.push_back(update.m_securityInventory.m_position.m_costBasis);
    row.push_back(update.m_securityInventory.m_position.m_key.m_currency);
    m_table.push(row);
  } else {
    m_table.transact([&] {
      m_table.set(index->second, Column::QUANTITY,
        Abs(update.m_securityInventory.m_position.m_quantity));
      m_table.set(index->second, Column::SIDE,
        GetSide(update.m_securityInventory.m_position.m_quantity));
      m_table.set(index->second, Column::AVERAGE_PRICE,
        GetAveragePrice(update.m_securityInventory.m_position));
      m_table.set(
        index->second, Column::PROFIT_AND_LOSS, update.m_unrealizedSecurity);
      m_table.set(index->second, Column::COST_BASIS,
        update.m_securityInventory.m_position.m_costBasis);
    });
  }
}
