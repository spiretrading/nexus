#include "Spire/Blotter/BlotterProfitAndLossModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Spire;

BlotterProfitAndLossModel::BlotterProfitAndLossModel(
    std::shared_ptr<PortfolioModel> portfolio)
    : m_portfolio(std::move(portfolio)),
      m_connection(m_portfolio->connect_update_signal(
        std::bind_front(&BlotterProfitAndLossModel::on_update, this))) {}

int BlotterProfitAndLossModel::get_row_size() const {
  return m_table.get_row_size();
}

int BlotterProfitAndLossModel::get_column_size() const {
  return COLUMN_SIZE;
}

AnyRef BlotterProfitAndLossModel::at(int row, int column) const {
  return m_table.at(row, column);
}

connection BlotterProfitAndLossModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_table.connect_operation_signal(slot);
}

void BlotterProfitAndLossModel::on_update(
    const PortfolioModel::Portfolio::UpdateEntry& update) {
  auto& security = update.m_securityInventory.m_position.m_key.m_index;
  auto index = m_indexes.find(security);
  if(index == m_indexes.end()) {
    m_indexes.insert(std::pair(security, m_table.get_row_size())).first;
    auto row = std::vector<std::any>();
    row.push_back(update.m_currencyInventory.m_position.m_key.m_currency);
    row.push_back(security);
    row.push_back(update.m_unrealizedSecurity +
      GetRealizedProfitAndLoss(update.m_securityInventory));
    row.push_back(update.m_securityInventory.m_fees);
    row.push_back(update.m_securityInventory.m_volume);
    m_table.push(row);
  } else {
    m_table.transact([&] {
      m_table.set(
        index->second, Column::PROFIT_AND_LOSS, update.m_unrealizedSecurity +
          GetRealizedProfitAndLoss(update.m_securityInventory));
      m_table.set(
        index->second, Column::FEES, update.m_securityInventory.m_fees);
      m_table.set(
        index->second, Column::VOLUME, update.m_securityInventory.m_volume);
    });
  }
}
