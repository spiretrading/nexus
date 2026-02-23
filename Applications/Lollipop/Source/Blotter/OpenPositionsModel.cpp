#include "Spire/Blotter/OpenPositionsModel.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

OpenPositionsModel::Entry::Entry(int index, const Ticker& ticker)
  : m_index(index),
    m_ticker(ticker),
    m_unrealizedEarnings(Money::ZERO) {}

OpenPositionsModel::OpenPositionsModel()
    : m_portfolioController(nullptr) {
  m_eventHandler.emplace();
}

void OpenPositionsModel::SetPortfolioController(
    Ref<ProfitAndLossModel::PortfolioController> portfolioController) {
  if(!m_entries.empty()) {
    beginRemoveRows(QModelIndex(), 0, m_entries.size() - 1);
    m_tickerToEntry.clear();
    m_entries.clear();
    endRemoveRows();
  }
  m_eventHandler = std::nullopt;
  m_eventHandler.emplace();
  m_portfolioController = portfolioController.get();
  m_portfolioController->get_publisher().monitor(
    m_eventHandler->get_slot<PortfolioUpdateEntry>(
      std::bind_front(&OpenPositionsModel::OnPortfolioUpdate, this)));
}

std::vector<OpenPositionsModel::Entry>
    OpenPositionsModel::GetOpenPositions() const {
  auto entries = std::vector<Entry>();
  for(auto i = m_entries.begin(); i != m_entries.end(); ++i) {
    entries.push_back(**i);
  }
  return entries;
}

boost::optional<const OpenPositionsModel::Entry&>
    OpenPositionsModel::GetOpenPosition(const Ticker& ticker) const {
  auto entryIterator = m_tickerToEntry.find(ticker);
  if(entryIterator == m_tickerToEntry.end()) {
    return none;
  }
  return *entryIterator->second;
}

int OpenPositionsModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_entries.size());
}

int OpenPositionsModel::columnCount(const QModelIndex& parent) const {
  return COLUMNS;
}

QVariant OpenPositionsModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto& entry = *m_entries[index.row()];
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(index.column() == TICKER_COLUMN) {
      return QVariant::fromValue(entry.m_ticker);
    } else if(index.column() == QUANTITY_COLUMN) {
      return QVariant::fromValue(abs(entry.m_inventory.m_position.m_quantity));
    } else if(index.column() == SIDE_COLUMN) {
      auto side = get_side(entry.m_inventory.m_position);
      return QVariant::fromValue(PositionSideToken(side));
    } else if(index.column() == AVERAGE_PRICE_COLUMN) {
      return QVariant::fromValue(
        get_average_price(entry.m_inventory.m_position));
    } else if(index.column() == PROFIT_LOSS_COLUMN) {
      return QVariant::fromValue(entry.m_unrealizedEarnings);
    } else if(index.column() == COST_BASIS_COLUMN) {
      return QVariant::fromValue(entry.m_inventory.m_position.m_cost_basis);
    } else if(index.column() == CURRENCY_COLUMN) {
      return QVariant::fromValue(entry.m_inventory.m_position.m_currency);
    }
  }
  return QVariant();
}

QVariant OpenPositionsModel::headerData(
    int section, Qt::Orientation orientation, int role) const {
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(section == TICKER_COLUMN) {
      return tr("Ticker");
    } else if(section == QUANTITY_COLUMN) {
      return tr("Qty");
    } else if(section == SIDE_COLUMN) {
      return tr("Side");
    } else if(section == AVERAGE_PRICE_COLUMN) {
      return tr("Avg. Price");
    } else if(section == PROFIT_LOSS_COLUMN) {
      return tr("P/L");
    } else if(section == COST_BASIS_COLUMN) {
      return tr("Cost Basis");
    } else if(section == CURRENCY_COLUMN) {
      return tr("Currency");
    }
  }
  return QVariant();
}

void OpenPositionsModel::OnPortfolioUpdate(const PortfolioUpdateEntry& update) {
  auto& ticker = update.m_inventory.m_position.m_ticker;
  auto entryIterator = m_tickerToEntry.find(ticker);
  if(entryIterator == m_tickerToEntry.end()) {
    if(update.m_inventory.m_position.m_quantity == 0) {
      return;
    }
    auto index = static_cast<int>(m_entries.size());
    beginInsertRows(QModelIndex(), index, index);
    auto entry = std::make_unique<Entry>(index, ticker);
    entryIterator =
      m_tickerToEntry.insert(std::pair(ticker, entry.get())).first;
    m_entries.emplace_back(std::move(entry));
    endInsertRows();
  }
  auto& entry = *entryIterator->second;
  entry.m_inventory = update.m_inventory;
  entry.m_unrealizedEarnings = update.m_unrealized;
  Q_EMIT dataChanged(
    index(entry.m_index, 0), index(entry.m_index, COLUMNS - 1));
  if(entry.m_inventory.m_position.m_quantity == 0) {
    beginRemoveRows(QModelIndex(), entry.m_index, entry.m_index);
    m_tickerToEntry.erase(ticker);
    for(auto i = entry.m_index + 1;
        i < static_cast<int>(m_entries.size()); ++i) {
      --m_entries[i]->m_index;
    }
    m_entries.erase(m_entries.begin() + entry.m_index);
    endRemoveRows();
  }
}
