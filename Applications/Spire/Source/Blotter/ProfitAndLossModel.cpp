#include "Spire/Blotter/ProfitAndLossModel.hpp"
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Spire/Blotter/ProfitAndLossEntryModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

ProfitAndLossModel::ProfitAndLossModel(
    Ref<const ExchangeRateTable> exchangeRates, bool showUnrealized)
    : m_exchangeRates(exchangeRates.get()),
      m_showUnrealized{showUnrealized},
      m_portfolioController(nullptr) {
  m_eventHandler.emplace();
}

ProfitAndLossModel::~ProfitAndLossModel() = default;

void ProfitAndLossModel::SetPortfolioController(
    Ref<PortfolioController> portfolioController) {
  for(auto& model : m_currencyToModel | boost::adaptors::map_values) {
    m_profitAndLossEntryModelRemovedSignal(*model);
    model.reset();
  }
  m_currencyToModel.clear();
  m_update = PortfolioUpdateEntry();
  m_update.m_currency_inventory.m_position.m_currency = m_currency;
  m_update.m_security_inventory.m_position.m_currency = m_currency;
  m_eventHandler = std::nullopt;
  m_eventHandler.emplace();
  m_portfolioController = portfolioController.get();
  m_portfolioController->get_publisher().monitor(
    m_eventHandler->get_slot<PortfolioUpdateEntry>(
      std::bind_front(&ProfitAndLossModel::OnPortfolioUpdate, this)));
  m_profitAndLossUpdateSignal(m_update);
}

void ProfitAndLossModel::SetCurrency(CurrencyId currency) {
  if(currency == m_currency) {
    return;
  }
  m_currency = currency;
  if(m_portfolioController) {
    SetPortfolioController(Ref(*m_portfolioController));
  } else {
    m_update = PortfolioUpdateEntry();
    m_update.m_currency_inventory.m_position.m_currency = m_currency;
    m_update.m_security_inventory.m_position.m_currency = m_currency;
  }
}

connection ProfitAndLossModel::ConnectProfitAndLossUpdateSignal(
    const ProfitAndLossUpdateSignal::slot_type& slot) const {
  slot(m_update);
  return m_profitAndLossUpdateSignal.connect(slot);
}

connection ProfitAndLossModel::ConnectProfitAndLossEntryModelAddedSignal(
    const ProfitAndLossEntryModelAddedSignal::slot_type& slot) const {
  return m_profitAndLossEntryModelAddedSignal.connect(slot);
}

connection ProfitAndLossModel::ConnectProfitAndLossEntryModelRemovedSignal(
    const ProfitAndLossEntryModelRemovedSignal::slot_type& slot) const {
  return m_profitAndLossEntryModelRemovedSignal.connect(slot);
}

void ProfitAndLossModel::OnPortfolioUpdate(const PortfolioUpdateEntry& update) {
  auto key = get_key(update.m_security_inventory.m_position);
  auto& model = m_currencyToModel[key.m_currency];
  if(!model) {
    model = std::make_unique<ProfitAndLossEntryModel>(
      DEFAULT_CURRENCIES.from(key.m_currency), m_showUnrealized);
    m_profitAndLossEntryModelAddedSignal(*model);
    m_currencyToPortfolio.insert(std::pair(key.m_currency, update));
  } else {
    auto& previous = m_currencyToPortfolio[key.m_currency];
    if(m_currency != CurrencyId::NONE) {
      m_update.m_currency_inventory.m_position.m_quantity -=
        previous.m_currency_inventory.m_position.m_quantity;
      m_update.m_currency_inventory.m_position.m_cost_basis -=
        m_exchangeRates->convert(
          previous.m_currency_inventory.m_position.m_cost_basis, key.m_currency,
          m_currency);
      m_update.m_currency_inventory.m_gross_profit_and_loss -=
        m_exchangeRates->convert(
          previous.m_currency_inventory.m_gross_profit_and_loss, key.m_currency,
          m_currency);
      m_update.m_currency_inventory.m_fees -= m_exchangeRates->convert(
        previous.m_currency_inventory.m_fees, key.m_currency, m_currency);
      m_update.m_currency_inventory.m_volume -=
        previous.m_currency_inventory.m_volume;
      m_update.m_currency_inventory.m_transaction_count -=
        previous.m_currency_inventory.m_transaction_count;
      m_update.m_unrealized_currency -= m_exchangeRates->convert(
        previous.m_unrealized_currency, key.m_currency, m_currency);
    }
    previous = update;
  }
  model->OnPortfolioUpdate(update);
  if(m_currency != CurrencyId::NONE) {
    m_update.m_currency_inventory.m_position.m_quantity +=
      update.m_currency_inventory.m_position.m_quantity;
    m_update.m_currency_inventory.m_position.m_cost_basis +=
      m_exchangeRates->convert(
        update.m_currency_inventory.m_position.m_cost_basis,
        key.m_currency, m_currency);
    m_update.m_currency_inventory.m_gross_profit_and_loss +=
      m_exchangeRates->convert(
        update.m_currency_inventory.m_gross_profit_and_loss,
        key.m_currency, m_currency);
    m_update.m_currency_inventory.m_fees += m_exchangeRates->convert(
      update.m_currency_inventory.m_fees, key.m_currency, m_currency);
    m_update.m_currency_inventory.m_volume +=
      update.m_currency_inventory.m_volume;
    m_update.m_currency_inventory.m_transaction_count +=
      update.m_currency_inventory.m_transaction_count;
    m_update.m_unrealized_currency += m_exchangeRates->convert(
      update.m_unrealized_currency, key.m_currency, m_currency);
    m_profitAndLossUpdateSignal(m_update);
  }
}
