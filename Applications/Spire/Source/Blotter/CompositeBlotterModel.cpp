#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/BlotterProfitAndLossModel.hpp"
#include "Spire/Blotter/LocalValuationModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/MultiSelectionModel.hpp"

using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

CompositeBlotterModel::CompositeBlotterModel(std::shared_ptr<TextModel> name,
    std::shared_ptr<BooleanModel> is_active,
    std::shared_ptr<BooleanModel> is_pinned, Nexus::MarketDatabase markets,
    Nexus::CurrencyId currency, Nexus::ExchangeRateTable exchange_rates,
    std::shared_ptr<MoneyModel> buying_power,
    std::shared_ptr<MoneyModel> net_loss,
    std::shared_ptr<BlotterTaskListModel> tasks,
    std::shared_ptr<ListModel<int>> task_selection,
    std::shared_ptr<ValuationModel> valuation,
    std::shared_ptr<ListModel<int>> position_selection)
  : m_name(std::move(name)),
    m_is_active(std::move(is_active)),
    m_is_pinned(std::move(is_pinned)),
    m_markets(std::move(markets)),
    m_currency(currency),
    m_exchange_rates(std::move(exchange_rates)),
    m_buying_power(std::move(buying_power)),
    m_net_loss(std::move(net_loss)),
    m_tasks(std::move(tasks)),
    m_task_selection(std::move(task_selection)),
    m_valuation(std::move(valuation)),
    m_position_selection(std::move(position_selection)) {}

std::shared_ptr<TextModel> CompositeBlotterModel::get_name() {
  return m_name;
}

std::shared_ptr<BooleanModel> CompositeBlotterModel::is_active() {
  return m_is_active;
}

std::shared_ptr<BooleanModel> CompositeBlotterModel::is_pinned() {
  return m_is_pinned;
}

const MarketDatabase& CompositeBlotterModel::get_markets() const {
  return m_markets;
}

CurrencyId CompositeBlotterModel::get_currency() const {
  return m_currency;
}

const ExchangeRateTable& CompositeBlotterModel::get_exchange_rates() const {
  return m_exchange_rates;
}

std::shared_ptr<MoneyModel> CompositeBlotterModel::get_buying_power() {
  return m_buying_power;
}

std::shared_ptr<MoneyModel> CompositeBlotterModel::get_net_loss() {
  return m_net_loss;
}

std::shared_ptr<BlotterTaskListModel> CompositeBlotterModel::get_tasks() {
  return m_tasks;
}

std::shared_ptr<ListModel<int>>
    CompositeBlotterModel::get_task_selection() const {
  return m_task_selection;
}

std::shared_ptr<ValuationModel> CompositeBlotterModel::get_valuation() {
  return m_valuation;
}

std::shared_ptr<ListModel<int>>
    CompositeBlotterModel::get_position_selection() const {
  return m_position_selection;
}

std::shared_ptr<CompositeBlotterModel> Spire::make_local_blotter_model(
    MarketDatabase markets, CurrencyId currency,
    ExchangeRateTable exchange_rates) {
  return std::make_shared<CompositeBlotterModel>(
    std::make_shared<LocalTextModel>(), std::make_shared<LocalBooleanModel>(),
    std::make_shared<LocalBooleanModel>(), std::move(markets), currency,
    std::move(exchange_rates), std::make_shared<LocalMoneyModel>(),
    std::make_shared<LocalMoneyModel>(),
    std::make_shared<ArrayListModel<std::shared_ptr<BlotterTaskEntry>>>(),
    std::make_shared<ListMultiSelectionModel>(),
    std::make_shared<LocalValuationModel>(),
    std::make_shared<ListMultiSelectionModel>());
}

std::shared_ptr<CompositeBlotterModel> Spire::make_derived_blotter_model(
    std::shared_ptr<TextModel> name, std::shared_ptr<BooleanModel> is_active,
    std::shared_ptr<BooleanModel> is_pinned, Nexus::MarketDatabase markets,
    Nexus::CurrencyId currency, Nexus::ExchangeRateTable exchange_rates,
    std::shared_ptr<MoneyModel> buying_power,
    std::shared_ptr<MoneyModel> net_loss,
    std::shared_ptr<BlotterTaskListModel> tasks,
    std::shared_ptr<ValuationModel> valuation) {
  return std::make_shared<CompositeBlotterModel>(std::move(name),
    std::move(is_active), std::move(is_pinned), std::move(markets), currency,
    std::move(exchange_rates), std::move(buying_power), std::move(net_loss),
    std::move(tasks), std::make_shared<ListMultiSelectionModel>(),
    std::move(valuation), std::make_shared<ListMultiSelectionModel>());
}
