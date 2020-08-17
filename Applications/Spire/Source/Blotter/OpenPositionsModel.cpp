#include "Spire/Blotter/OpenPositionsModel.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  const unsigned int UPDATE_INTERVAL = 100;
}

OpenPositionsModel::Entry::Entry(int index, const SpireBookkeeper::Key& key)
    : m_index(index),
      m_key(key),
      m_unrealizedEarnings(Money::ZERO) {}

OpenPositionsModel::OpenPositionsModel()
    : m_portfolioController(nullptr) {
  m_slotHandler.emplace();
  connect(&m_updateTimer, &QTimer::timeout, this,
    &OpenPositionsModel::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
}

OpenPositionsModel::~OpenPositionsModel() {}

void OpenPositionsModel::SetPortfolioController(
    Ref<SpirePortfolioController> portfolioController) {
  if(!m_entries.empty()) {
    beginRemoveRows(QModelIndex(), 0, m_entries.size() - 1);
    m_securityToEntry.clear();
    m_entries.clear();
    endRemoveRows();
  }
  m_slotHandler = std::nullopt;
  m_slotHandler.emplace();
  m_portfolioController = portfolioController.Get();
  m_portfolioController->GetPublisher().Monitor(
    m_slotHandler->GetSlot<SpirePortfolioController::UpdateEntry>(
    std::bind(&OpenPositionsModel::OnPortfolioUpdate, this,
    std::placeholders::_1)));
}

vector<OpenPositionsModel::Entry> OpenPositionsModel::GetOpenPositions() const {
  vector<Entry> entries;
  for(auto i = m_entries.begin(); i != m_entries.end(); ++i) {
    entries.push_back(**i);
  }
  return entries;
}

boost::optional<const OpenPositionsModel::Entry&>
    OpenPositionsModel::GetOpenPosition(const Security& security) const {
  auto entryIterator = m_securityToEntry.find(security);
  if(entryIterator == m_securityToEntry.end()) {
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
  Entry& entry = *m_entries[index.row()];
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(index.column() == SECURITY_COLUMN) {
      return QVariant::fromValue(entry.m_key.m_index);
    } else if(index.column() == QUANTITY_COLUMN) {
      return QVariant::fromValue(Abs(entry.m_inventory.m_position.m_quantity));
    } else if(index.column() == SIDE_COLUMN) {
      Side side = GetSide(entry.m_inventory.m_position);
      return QVariant::fromValue(PositionSideToken(side));
    } else if(index.column() == AVERAGE_PRICE_COLUMN) {
      return QVariant::fromValue(GetAveragePrice(entry.m_inventory.m_position));
    } else if(index.column() == PROFIT_LOSS_COLUMN) {
      return QVariant::fromValue(entry.m_unrealizedEarnings);
    } else if(index.column() == COST_BASIS_COLUMN) {
      return QVariant::fromValue(entry.m_inventory.m_position.m_costBasis);
    } else if(index.column() == CURRENCY_COLUMN) {
      return QVariant::fromValue(entry.m_key.m_currency);
    }
  }
  return QVariant();
}

QVariant OpenPositionsModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(section == SECURITY_COLUMN) {
      return tr("Security");
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

void OpenPositionsModel::OnUpdateTimer() {
  HandleTasks(*m_slotHandler);
}

void OpenPositionsModel::OnPortfolioUpdate(
    const SpirePortfolioController::UpdateEntry& update) {
  const SpirePosition::Key& key = update.m_securityInventory.m_position.m_key;
  auto entryIterator = m_securityToEntry.find(key.m_index);
  if(entryIterator == m_securityToEntry.end()) {
    int index = static_cast<int>(m_entries.size());
    beginInsertRows(QModelIndex(), index, index);
    unique_ptr<Entry> entry = std::make_unique<Entry>(index, key);
    entryIterator = m_securityToEntry.insert(
      make_pair(key.m_index, entry.get())).first;
    m_entries.emplace_back(std::move(entry));
    endInsertRows();
  }
  Entry& entry = *entryIterator->second;
  entry.m_inventory = update.m_securityInventory;
  entry.m_unrealizedEarnings = update.m_unrealizedSecurity;
  Q_EMIT dataChanged(index(entry.m_index, 0),
    index(entry.m_index, COLUMNS - 1));
  if(entry.m_inventory.m_position.m_quantity == 0) {
    beginRemoveRows(QModelIndex(), entry.m_index, entry.m_index);
    m_securityToEntry.erase(key.m_index);
    for(int i = entry.m_index + 1;
        i < static_cast<int>(m_entries.size()); ++i) {
      --m_entries[i]->m_index;
    }
    m_entries.erase(m_entries.begin() + entry.m_index);
    endRemoveRows();
  }
}
