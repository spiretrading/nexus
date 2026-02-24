#include "Spire/PortfolioViewer/PortfolioViewerModel.hpp"
#include <QCoreApplication>
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Spire/PortfolioViewer/PortfolioSelectionModel.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

PortfolioViewerModel::Entry::Entry(const DirectoryEntry& account)
  : m_account(account) {}

PortfolioViewerModel::TotalEntry::TotalEntry()
  : m_volume(0),
    m_trades(0) {}

PortfolioViewerModel::PortfolioViewerModel(Ref<UserProfile> userProfile,
    Ref<PortfolioSelectionModel> selectionModel)
    : m_userProfile(userProfile.get()),
      m_totalCurrency(DefaultCurrencies::CAD),
      m_exchangeRates(&m_userProfile->GetExchangeRates()),
      m_selectionModel(selectionModel.get()),
      m_displayCount(0) {
  m_userProfile->GetClients().get_risk_client().
    get_risk_portfolio_update_publisher().monitor(
      m_eventHandler.get_slot<RiskInventoryEntry>(std::bind_front(
        &PortfolioViewerModel::OnRiskPortfolioInventoryUpdate, this)));
  connect(m_selectionModel, &PortfolioSelectionModel::dataChanged, this,
    &PortfolioViewerModel::OnSelectionModelUpdated);
}

const PortfolioViewerModel::Entry&
    PortfolioViewerModel::GetEntry(int index) const {
  auto i = index;
  while(i < static_cast<int>(m_entries.size())) {
    auto& entry = m_entries[i];
    if(entry.m_displayIndex == index && entry.m_isDisplayed) {
      return entry;
    }
    ++i;
  }
  assert(false);
  return m_entries.back();
}

const PortfolioViewerModel::TotalEntry&
    PortfolioViewerModel::GetTotals() const {
  return m_totals;
}

connection PortfolioViewerModel::ConnectTotalsUpdatedSignal(
    const TotalsUpdatedSignal::slot_type& slot) const {
  return m_totalsUpdatedSignal.connect(slot);
}

int PortfolioViewerModel::rowCount(const QModelIndex& parent) const {
  return m_displayCount;
}

int PortfolioViewerModel::columnCount(const QModelIndex& parent) const {
  return COLUMN_COUNT;
}

QVariant PortfolioViewerModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto& entry = GetEntry(index.row());
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(index.column() == GROUP_COLUMN) {
      auto groupIterator = m_groups.find(entry.m_account);
      if(groupIterator == m_groups.end()) {
        return tr("N/A");
      }
      return QString::fromStdString(groupIterator->second.m_name);
    } else if(index.column() == ACCOUNT_COLUMN) {
      return QString::fromStdString(entry.m_account.m_name);
    } else if(index.column() == ACCOUNT_TOTAL_PROFIT_LOSS_COLUMN) {
      auto accountTotalsIterator = m_accountTotals.find(entry.m_account);
      if(accountTotalsIterator == m_accountTotals.end()) {
        return tr("N/A");
      }
      return QVariant::fromValue(
        accountTotalsIterator->second.m_realizedProfitAndLoss +
        accountTotalsIterator->second.m_unrealizedProfitAndLoss);
    } else if(index.column() == ACCOUNT_UNREALIZED_PROFIT_LOSS_COLUMN) {
      auto accountTotalsIterator = m_accountTotals.find(entry.m_account);
      if(accountTotalsIterator == m_accountTotals.end()) {
        return tr("N/A");
      }
      return QVariant::fromValue(
        accountTotalsIterator->second.m_unrealizedProfitAndLoss);
    } else if(index.column() == ACCOUNT_REALIZED_PROFIT_LOSS_COLUMN) {
      auto accountTotalsIterator = m_accountTotals.find(entry.m_account);
      if(accountTotalsIterator == m_accountTotals.end()) {
        return tr("N/A");
      }
      return QVariant::fromValue(
        accountTotalsIterator->second.m_realizedProfitAndLoss);
    } else if(index.column() == ACCOUNT_FEES_COLUMN) {
      auto accountTotalsIterator = m_accountTotals.find(entry.m_account);
      if(accountTotalsIterator == m_accountTotals.end()) {
        return tr("N/A");
      }
      return QVariant::fromValue(accountTotalsIterator->second.m_fees);
    } else if(index.column() == TICKER_COLUMN) {
      return QVariant::fromValue(entry.m_inventory.m_position.m_ticker);
    } else if(index.column() == OPEN_QUANTITY_COLUMN) {
      return QVariant::fromValue(abs(entry.m_inventory.m_position.m_quantity));
    } else if(index.column() == SIDE_COLUMN) {
      return QVariant::fromValue(PositionSideToken(
        get_side(entry.m_inventory.m_position)));
    } else if(index.column() == AVERAGE_PRICE_COLUMN) {
      return QVariant::fromValue(
        get_average_price(entry.m_inventory.m_position));
    } else if(index.column() == TOTAL_PROFIT_LOSS_COLUMN) {
      auto unrealizedProfitAndLoss =
        GetUnrealizedProfitAndLoss(entry.m_inventory);
      if(!unrealizedProfitAndLoss) {
        return tr("N/A");
      }
      auto totalProfitAndLoss = *unrealizedProfitAndLoss +
        get_realized_profit_and_loss(entry.m_inventory);
      return QVariant::fromValue(totalProfitAndLoss);
    } else if(index.column() == UNREALIZED_PROFIT_LOSS_COLUMN) {
      auto unrealizedProfitAndLoss =
        GetUnrealizedProfitAndLoss(entry.m_inventory);
      if(!unrealizedProfitAndLoss) {
        return tr("N/A");
      }
      return QVariant::fromValue(*unrealizedProfitAndLoss);
    } else if(index.column() == REALIZED_PROFIT_LOSS_COLUMN) {
      return QVariant::fromValue(
        get_realized_profit_and_loss(entry.m_inventory));
    } else if(index.column() == FEES_COLUMN) {
      return QVariant::fromValue(entry.m_inventory.m_fees);
    } else if(index.column() == COST_BASIS_COLUMN) {
      return QVariant::fromValue(entry.m_inventory.m_position.m_cost_basis);
    } else if(index.column() == CURRENCY_COLUMN) {
      return QVariant::fromValue(entry.m_inventory.m_position.m_currency);
    } else if(index.column() == VOLUME_COLUMN) {
      return QVariant::fromValue(entry.m_inventory.m_volume);
    } else if(index.column() == TRADES_COLUMN) {
      return QVariant::fromValue(entry.m_inventory.m_transaction_count);
    }
  }
  return QVariant();
}

QVariant PortfolioViewerModel::headerData(
    int section, Qt::Orientation orientation, int role) const {
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(section == GROUP_COLUMN) {
      return tr("Group");
    } else if(section == ACCOUNT_COLUMN) {
      return tr("Account");
    } else if(section == ACCOUNT_TOTAL_PROFIT_LOSS_COLUMN) {
      return tr("Acc. Total P/L");
    } else if(section == ACCOUNT_UNREALIZED_PROFIT_LOSS_COLUMN) {
      return tr("Acc. Unrealized");
    } else if(section == ACCOUNT_REALIZED_PROFIT_LOSS_COLUMN) {
      return tr("Acc. Realized");
    } else if(section == ACCOUNT_FEES_COLUMN) {
      return tr("Acc. Fees");
    } else if(section == TICKER_COLUMN) {
      return tr("Ticker");
    } else if(section == OPEN_QUANTITY_COLUMN) {
      return tr("Open Quantity");
    } else if(section == SIDE_COLUMN) {
      return tr("Side");
    } else if(section == AVERAGE_PRICE_COLUMN) {
      return tr("Average Price");
    } else if(section == TOTAL_PROFIT_LOSS_COLUMN) {
      return tr("Total P/L");
    } else if(section == UNREALIZED_PROFIT_LOSS_COLUMN) {
      return tr("Unrealized");
    } else if(section == REALIZED_PROFIT_LOSS_COLUMN) {
      return tr("Realized");
    } else if(section == FEES_COLUMN) {
      return tr("Fees");
    } else if(section == COST_BASIS_COLUMN) {
      return tr("Cost Basis");
    } else if(section == CURRENCY_COLUMN) {
      return tr("Currency");
    } else if(section == VOLUME_COLUMN) {
      return tr("Volume");
    } else if(section == TRADES_COLUMN) {
      return tr("Trades");
    }
  }
  return QVariant();
}

boost::optional<Money> PortfolioViewerModel::GetUnrealizedProfitAndLoss(
    const Inventory& inventory) const {
  auto valuationIterator = m_valuations.find(inventory.m_position.m_ticker);
  if(valuationIterator == m_valuations.end()) {
    return none;
  }
  auto& valuation = valuationIterator->second;
  auto unrealizedProfitAndLoss = get_unrealized_profit_and_loss(
    inventory, valuation);
  if(!unrealizedProfitAndLoss) {
    return none;
  }
  return *unrealizedProfitAndLoss;
}

void PortfolioViewerModel::OnBboQuote(
    const Ticker& ticker, const BboQuote& bboQuote) {
  auto& valuation = m_valuations.at(ticker);
  auto previousValuation = valuation;
  valuation.m_ask_value = bboQuote.m_ask.m_price;
  valuation.m_bid_value = bboQuote.m_bid.m_price;
  auto& indexes = m_tickerToIndexes.at(ticker);
  for(auto& item : indexes) {
    auto& viewerEntry = m_entries[item];
    auto baseCurrency = viewerEntry.m_inventory.m_position.m_currency;
    auto& accountTotals = m_accountTotals[viewerEntry.m_account];
    auto unrealizedProfitAndLoss = get_unrealized_profit_and_loss(
      viewerEntry.m_inventory, previousValuation);
    if(unrealizedProfitAndLoss) {
      accountTotals.m_unrealizedProfitAndLoss -= m_exchangeRates->convert(
        *unrealizedProfitAndLoss, baseCurrency, m_totalCurrency);
      if(viewerEntry.m_isDisplayed) {
        m_totals.m_unrealizedProfitAndLoss -= m_exchangeRates->convert(
          *unrealizedProfitAndLoss, baseCurrency, m_totalCurrency);
      }
    }
    unrealizedProfitAndLoss = GetUnrealizedProfitAndLoss(
      viewerEntry.m_inventory);
    if(unrealizedProfitAndLoss) {
      accountTotals.m_unrealizedProfitAndLoss += m_exchangeRates->convert(
        *unrealizedProfitAndLoss, baseCurrency, m_totalCurrency);
      if(viewerEntry.m_isDisplayed) {
        m_totals.m_unrealizedProfitAndLoss += m_exchangeRates->convert(
          *unrealizedProfitAndLoss, baseCurrency, m_totalCurrency);
      }
    }
    if(viewerEntry.m_isDisplayed) {
      Q_EMIT dataChanged(index(viewerEntry.m_displayIndex, 0),
        index(viewerEntry.m_displayIndex, COLUMN_COUNT - 1));
      m_totalsUpdatedSignal(m_totals);
    }
  }
}

void PortfolioViewerModel::OnRiskPortfolioInventoryUpdate(
    const RiskInventoryEntry& entry) {
  auto& ticker = entry.m_key.m_ticker;
  auto baseCurrency = entry.m_value.m_position.m_currency;
  if(m_valuations.find(ticker) == m_valuations.end()) {
    m_valuations.insert(
      std::pair(ticker, Valuation(entry.m_value.m_position.m_currency)));
    auto bboQuery = make_current_query(ticker);
    bboQuery.set_interruption_policy(InterruptionPolicy::IGNORE_CONTINUE);
    m_userProfile->GetClients().get_market_data_client().query(
      bboQuery, m_eventHandler.get_slot<BboQuote>(
        std::bind_front(&PortfolioViewerModel::OnBboQuote, this, ticker)));
  }
  auto indexIterator = m_inventoryKeyToIndex.find(entry.m_key);
  if(indexIterator == m_inventoryKeyToIndex.end()) {
    auto portfolioViewerEntry = IndexedEntry();
    portfolioViewerEntry.m_account = entry.m_key.m_account;
    portfolioViewerEntry.m_inventory = entry.m_value;
    auto row = static_cast<int>(m_entries.size());
    auto groupIterator = m_groups.find(entry.m_key.m_account);
    if(groupIterator == m_groups.end()) {
      auto group = m_userProfile->GetClients().get_administration_client().
        load_parent_trading_group(entry.m_key.m_account);
      groupIterator =
        m_groups.insert(std::pair(entry.m_key.m_account, group)).first;
    }
    portfolioViewerEntry.m_group = groupIterator->second;
    auto& accountTotals = m_accountTotals[entry.m_key.m_account];
    auto realizedProfitAndLoss = get_realized_profit_and_loss(entry.m_value);
    accountTotals.m_realizedProfitAndLoss += m_exchangeRates->convert(
      realizedProfitAndLoss, baseCurrency, m_totalCurrency);
    accountTotals.m_fees += m_exchangeRates->convert(
      entry.m_value.m_fees, baseCurrency, m_totalCurrency);
    auto unrealizedProfitAndLoss = GetUnrealizedProfitAndLoss(entry.m_value);
    if(unrealizedProfitAndLoss) {
      accountTotals.m_unrealizedProfitAndLoss += m_exchangeRates->convert(
        *unrealizedProfitAndLoss, baseCurrency, m_totalCurrency);
    }
    portfolioViewerEntry.m_isDisplayed =
      m_selectionModel->TestSelectionAcceptsEntry(portfolioViewerEntry);
    if(m_entries.empty()) {
      portfolioViewerEntry.m_displayIndex = 0;
    } else {
      portfolioViewerEntry.m_displayIndex = m_entries.back().m_displayIndex;
      if(m_entries.back().m_isDisplayed) {
        ++portfolioViewerEntry.m_displayIndex;
      }
    }
    if(portfolioViewerEntry.m_isDisplayed) {
      beginInsertRows(QModelIndex(), portfolioViewerEntry.m_displayIndex,
        portfolioViewerEntry.m_displayIndex);
    }
    m_entries.push_back(portfolioViewerEntry);
    m_inventoryKeyToIndex.insert(std::make_pair(entry.m_key, row));
    m_tickerToIndexes[entry.m_key.m_ticker].push_back(row);
    if(portfolioViewerEntry.m_isDisplayed) {
      ++m_displayCount;
      endInsertRows();
      m_totals.m_trades += entry.m_value.m_transaction_count;
      m_totals.m_volume += entry.m_value.m_volume;
      m_totals.m_realizedProfitAndLoss += m_exchangeRates->convert(
        realizedProfitAndLoss, baseCurrency, m_totalCurrency);
      m_totals.m_fees += m_exchangeRates->convert(entry.m_value.m_fees,
        baseCurrency, m_totalCurrency);
      if(unrealizedProfitAndLoss) {
        m_totals.m_unrealizedProfitAndLoss += m_exchangeRates->convert(
          *unrealizedProfitAndLoss, baseCurrency, m_totalCurrency);
      }
      m_totalsUpdatedSignal(m_totals);
    }
  } else {
    auto row = indexIterator->second;
    auto& viewerEntry = m_entries[row];
    auto& accountTotals = m_accountTotals[entry.m_key.m_account];
    accountTotals.m_realizedProfitAndLoss -= m_exchangeRates->convert(
      get_realized_profit_and_loss(viewerEntry.m_inventory), baseCurrency,
      m_totalCurrency);
    accountTotals.m_fees -= m_exchangeRates->convert(
      viewerEntry.m_inventory.m_fees, baseCurrency, m_totalCurrency);
    auto unrealizedProfitAndLoss = GetUnrealizedProfitAndLoss(
      viewerEntry.m_inventory);
    if(unrealizedProfitAndLoss) {
      accountTotals.m_unrealizedProfitAndLoss -= m_exchangeRates->convert(
        *unrealizedProfitAndLoss, baseCurrency, m_totalCurrency);
      if(viewerEntry.m_isDisplayed) {
        m_totals.m_unrealizedProfitAndLoss -= m_exchangeRates->convert(
          *unrealizedProfitAndLoss, baseCurrency, m_totalCurrency);
      }
    }
    if(viewerEntry.m_isDisplayed) {
      m_totals.m_trades -= viewerEntry.m_inventory.m_transaction_count;
      m_totals.m_volume -= viewerEntry.m_inventory.m_volume;
      m_totals.m_realizedProfitAndLoss -= m_exchangeRates->convert(
        get_realized_profit_and_loss(viewerEntry.m_inventory), baseCurrency,
        m_totalCurrency);
      m_totals.m_fees -= m_exchangeRates->convert(
        viewerEntry.m_inventory.m_fees, baseCurrency, m_totalCurrency);
    }
    viewerEntry.m_inventory = entry.m_value;
    auto realizedProfitAndLoss = get_realized_profit_and_loss(
      viewerEntry.m_inventory);
    accountTotals.m_realizedProfitAndLoss += m_exchangeRates->convert(
      realizedProfitAndLoss, baseCurrency, m_totalCurrency);
    accountTotals.m_fees += m_exchangeRates->convert(
      viewerEntry.m_inventory.m_fees, baseCurrency, m_totalCurrency);
    unrealizedProfitAndLoss = GetUnrealizedProfitAndLoss(
      viewerEntry.m_inventory);
    if(unrealizedProfitAndLoss) {
      accountTotals.m_unrealizedProfitAndLoss += m_exchangeRates->convert(
        *unrealizedProfitAndLoss, baseCurrency, m_totalCurrency);
    }
    auto previousVisibility = viewerEntry.m_isDisplayed;
    viewerEntry.m_isDisplayed = m_selectionModel->TestSelectionAcceptsEntry(
      viewerEntry);
    if(previousVisibility != viewerEntry.m_isDisplayed) {
      if(viewerEntry.m_isDisplayed) {
        beginInsertRows(QModelIndex(), viewerEntry.m_displayIndex,
          viewerEntry.m_displayIndex);
        for(auto i = row + 1; i < static_cast<int>(m_entries.size()); ++i) {
          ++m_entries[i].m_displayIndex;
        }
        ++m_displayCount;
        endInsertRows();
      } else {
        beginRemoveRows(QModelIndex(), viewerEntry.m_displayIndex,
          viewerEntry.m_displayIndex);
        for(auto i = row + 1; i < static_cast<int>(m_entries.size()); ++i) {
          --m_entries[i].m_displayIndex;
        }
        --m_displayCount;
        endRemoveRows();
      }
    } else if(viewerEntry.m_isDisplayed) {
      Q_EMIT dataChanged(index(viewerEntry.m_displayIndex, 0),
        index(viewerEntry.m_displayIndex, COLUMN_COUNT - 1));
    }
    if(viewerEntry.m_isDisplayed) {
      m_totals.m_trades += entry.m_value.m_transaction_count;
      m_totals.m_volume += entry.m_value.m_volume;
      m_totals.m_realizedProfitAndLoss += m_exchangeRates->convert(
        realizedProfitAndLoss, baseCurrency, m_totalCurrency);
      m_totals.m_fees += m_exchangeRates->convert(entry.m_value.m_fees,
        baseCurrency, m_totalCurrency);
      if(unrealizedProfitAndLoss) {
        m_totals.m_unrealizedProfitAndLoss += m_exchangeRates->convert(
          *unrealizedProfitAndLoss, baseCurrency, m_totalCurrency);
      }
      m_totalsUpdatedSignal(m_totals);
    }
    if(entry.m_value.m_transaction_count == 0 &&
        entry.m_value.m_position.m_quantity == 0) {
      if(viewerEntry.m_isDisplayed) {
        beginRemoveRows(QModelIndex(), viewerEntry.m_displayIndex,
          viewerEntry.m_displayIndex);
        for(auto i = row + 1; i < static_cast<int>(m_entries.size()); ++i) {
          --m_entries[i].m_displayIndex;
        }
        --m_displayCount;
        endRemoveRows();
      }
      auto& tickerIndicies =
        m_tickerToIndexes[viewerEntry.m_inventory.m_position.m_ticker];
      auto tickerIndexIterator = std::find(tickerIndicies.begin(),
        tickerIndicies.end(), row);
      tickerIndicies.erase(tickerIndexIterator);
      for(auto& tickers : m_tickerToIndexes) {
        for(auto& index : tickers.second) {
          if(index > row) {
            --index;
          }
        }
      }
      m_inventoryKeyToIndex.erase(entry.m_key);
      for(auto& index : m_inventoryKeyToIndex) {
        if(index.second > row) {
          --index.second;
        }
      }
      m_entries.erase(m_entries.begin() + row);
    }
  }
}

void PortfolioViewerModel::OnSelectionModelUpdated(const QModelIndex& topLeft,
    const QModelIndex& bottomRight) {
  beginResetModel();
  m_displayCount = 0;
  m_totals = TotalEntry();
  for(auto& entry : m_entries) {
    if(m_selectionModel->TestSelectionAcceptsEntry(entry)) {
      auto baseCurrency = entry.m_inventory.m_position.m_currency;
      entry.m_isDisplayed = true;
      entry.m_displayIndex = m_displayCount;
      ++m_displayCount;
      m_totals.m_trades += entry.m_inventory.m_transaction_count;
      m_totals.m_volume += entry.m_inventory.m_volume;
      m_totals.m_realizedProfitAndLoss += m_exchangeRates->convert(
        get_realized_profit_and_loss(entry.m_inventory), baseCurrency,
        m_totalCurrency);
      m_totals.m_fees += m_exchangeRates->convert(
        entry.m_inventory.m_fees, baseCurrency, m_totalCurrency);
      auto unrealizedProfitAndLoss =
        GetUnrealizedProfitAndLoss(entry.m_inventory);
      if(unrealizedProfitAndLoss) {
        m_totals.m_unrealizedProfitAndLoss += m_exchangeRates->convert(
          *unrealizedProfitAndLoss, baseCurrency, m_totalCurrency);
      }
    } else {
      entry.m_isDisplayed = false;
      entry.m_displayIndex = m_displayCount;
    }
  }
  endResetModel();
  m_totalsUpdatedSignal(m_totals);
}
