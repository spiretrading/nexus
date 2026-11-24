#include "Spire/Blotter/PortfolioSummaryModel.hpp"
#include <tuple>
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

PortfolioSummaryModel::PortfolioSummaryModel(
    std::shared_ptr<PortfolioModel> portfolio, CurrencyId currency,
    const ExchangeRateTable& rates)
    : m_portfolio(std::move(portfolio)),
      m_rates(rates),
      m_depth(0),
      m_is_rebuild(true) {
  m_value.m_currency_inventory.m_position.m_currency = currency;
  update();
  m_connection = m_portfolio->connect_operation_signal(
    std::bind_front(&PortfolioSummaryModel::on_operation, this));
}

const PortfolioSummaryModel::Type& PortfolioSummaryModel::get() const {
  return m_value;
}

connection PortfolioSummaryModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

void PortfolioSummaryModel::rebuild() {
  auto inventories = std::vector<Inventory>();
  inventories.reserve(m_portfolio->get_size());
  m_unrealized.clear();
  for(auto& entry : std::as_const(*m_portfolio)) {
    inventories.push_back(entry.m_inventory);
    m_unrealized[entry.m_inventory.m_position.m_currency] += entry.m_unrealized;
  }
  auto bookkeeper = TrueAverageBookkeeper(inventories);
  auto totals = bookkeeper.get_totals_range();
  m_totals.assign(totals.begin(), totals.end());
  m_is_rebuild = false;
}

void PortfolioSummaryModel::update() {
  if(m_depth != 0) {
    return;
  }
  if(m_is_rebuild) {
    rebuild();
  }
  auto value = Type();
  auto& inventory = value.m_currency_inventory;
  inventory.m_position.m_currency =
    m_value.m_currency_inventory.m_position.m_currency;
  for(auto& totals : m_totals) {
    auto currency = totals.m_position.m_currency;
    auto convert = [&] (Money amount) {
      return m_rates.convert(amount, currency, inventory.m_position.m_currency);
    };
    inventory.m_gross_profit_and_loss +=
      convert(totals.m_gross_profit_and_loss);
    inventory.m_fees += convert(totals.m_fees);
    inventory.m_position.m_cost_basis +=
      convert(totals.m_position.m_cost_basis);
    inventory.m_position.m_quantity += totals.m_position.m_quantity;
    inventory.m_volume += totals.m_volume;
    inventory.m_transaction_count += totals.m_transaction_count;
    value.m_currency_unrealized += convert(m_unrealized.at(currency));
  }
  if(std::tie(value.m_currency_inventory, value.m_currency_unrealized) !=
      std::tie(m_value.m_currency_inventory, m_value.m_currency_unrealized)) {
    m_value = value;
    m_update_signal(m_value);
  }
}

void PortfolioSummaryModel::on_operation(
    const PortfolioModel::Operation& operation) {
  visit(operation,
    [&] (PortfolioModel::StartTransaction) {
      ++m_depth;
    },
    [&] (PortfolioModel::EndTransaction) {
      --m_depth;
      update();
    },
    [&] (const PortfolioModel::AddOperation&) {
      m_is_rebuild = true;
      update();
    },
    [&] (const PortfolioModel::RemoveOperation&) {
      m_is_rebuild = true;
      update();
    },
    [&] (const PortfolioModel::UpdateOperation& change) {
      auto& previous = change.get_previous();
      auto& value = change.get_value();
      if(previous.m_inventory != value.m_inventory) {
        m_is_rebuild = true;
      } else if(!m_is_rebuild) {
        m_unrealized[value.m_inventory.m_position.m_currency] +=
          value.m_unrealized - previous.m_unrealized;
      }
      update();
    });
}
