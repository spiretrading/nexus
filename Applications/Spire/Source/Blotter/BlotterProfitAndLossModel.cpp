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

QValidator::State BlotterProfitAndLossModel::set(
    int row, int column, const std::any& value) {
  if(column != Column::CURRENCY || value.type() != typeid(CurrencyIndex)) {
    return QValidator::State::Invalid;
  }
  auto& index = std::any_cast<const CurrencyIndex&>(value);
  auto& current_index = m_table.get<CurrencyIndex>(row, column);
  if(current_index.m_index != index.m_index) {
    return QValidator::State::Invalid;
  }
  if(current_index.m_is_expanded == index.m_is_expanded) {
    return QValidator::State::Acceptable;
  }
  m_table.transact([&] {
    m_table.set(row, column, value);
    if(index.m_is_expanded) {
      for(auto& inventory :
          m_portfolio->get_portfolio().GetBookkeeper().GetInventoryRange()) {
        if(inventory.m_position.m_key.m_currency == index.m_index) {
          auto& security = inventory.m_position.m_key.m_index;
          auto& entry =
            m_portfolio->get_portfolio().GetSecurityEntries().at(security);
          update(security, entry.m_unrealized, inventory);
        }
      }
    } else {
      auto i = 0;
      while(i != m_table.get_row_size()) {
        auto& security = m_table.get<Security>(i, Column::SECURITY);
        if(m_table.get<CurrencyIndex>(i, Column::CURRENCY).m_index ==
            index.m_index && security != Security()) {
          m_table.remove(i);
          m_indexes.erase(security);
        } else {
          ++i;
        }
      }
    }
  });
  return QValidator::State::Acceptable;
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
    auto currency = inventory.m_position.m_key.m_currency;
    if(index.type() == typeid(Security)) {
      auto& i = get<CurrencyIndex>(m_indexes.at(currency), Column::CURRENCY);
      if(!i.m_is_expanded) {
        return;
      }
    }
    m_indexes.insert(std::pair(index, m_table.get_row_size())).first;
    auto row = std::vector<std::any>();
    row.push_back(CurrencyIndex(currency, false));
    if(index.type() == typeid(CurrencyId)) {
      row.push_back(Security());
    } else {
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
    update(security, entry.m_unrealizedSecurity, entry.m_securityInventory);
  });
}
