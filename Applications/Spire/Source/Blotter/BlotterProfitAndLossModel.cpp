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

void BlotterProfitAndLossModel::update(const Index& index,
    Money unrealized_profit_and_loss,
    const PortfolioModel::Portfolio::UpdateEntry::Inventory& inventory) {
  auto i = m_indexes.find(index);
  if(i == m_indexes.end()) {
    m_indexes.insert(std::pair(index, m_table.get_row_size())).first;
    auto row = std::vector<std::any>();
    if(index.type() == typeid(CurrencyId)) {
      row.push_back(
        CurrencyIndex(inventory.m_position.m_key.m_currency, false));
      row.push_back(Security());
    } else {
      row.push_back(inventory.m_position.m_key.m_currency);
      row.push_back(boost::get<Security>(index));
    }
    row.push_back(
      unrealized_profit_and_loss + GetRealizedProfitAndLoss(inventory));
    row.push_back(inventory.m_fees);
    row.push_back(inventory.m_volume);
    m_table.push(row);
  } else {
    m_table.set(i->second, Column::PROFIT_AND_LOSS,
      unrealized_profit_and_loss + GetRealizedProfitAndLoss(inventory));
    m_table.set(i->second, Column::FEES, inventory.m_fees);
    m_table.set(i->second, Column::VOLUME, inventory.m_volume);
  }
}

void BlotterProfitAndLossModel::on_update(
    const PortfolioModel::Portfolio::UpdateEntry& entry) {
  auto& currency = entry.m_currencyInventory.m_position.m_key.m_currency;
  auto& security = entry.m_securityInventory.m_position.m_key.m_index;
  m_table.transact([&] {
    update(currency, entry.m_unrealizedCurrency, entry.m_currencyInventory);
    update(security, entry.m_unrealizedSecurity,
      entry.m_securityInventory);
  });
}
