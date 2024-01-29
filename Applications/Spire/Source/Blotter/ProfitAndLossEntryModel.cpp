#include "Spire/Blotter/ProfitAndLossEntryModel.hpp"
#include <Beam/Utilities/Algorithm.hpp>
#include "Spire/LegacyUI/CustomQtVariants.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

ProfitAndLossEntryModel::Entry::Entry(const Security& security)
    : m_security{security},
      m_volume{0},
      m_previousQuantity{0} {}

ProfitAndLossEntryModel::ProfitAndLossEntryModel(
    const CurrencyDatabase::Entry& currency, bool showUnrealized)
    : m_currency(currency),
      m_showUnrealized{showUnrealized},
      m_volume{0} {}

ProfitAndLossEntryModel::~ProfitAndLossEntryModel() {}

const CurrencyDatabase::Entry& ProfitAndLossEntryModel::GetCurrency() const {
  return m_currency;
}

void ProfitAndLossEntryModel::OnPortfolioUpdate(
    const SpirePortfolioController::UpdateEntry& update) {
  const SpirePosition::Key& key = update.m_securityInventory.m_position.m_key;
  Money currencyProfitAndLoss =
    update.m_currencyInventory.m_grossProfitAndLoss -
    update.m_currencyInventory.m_fees;
  if(m_showUnrealized) {
    currencyProfitAndLoss += update.m_unrealizedCurrency;
  }
  m_profitAndLossSignal(currencyProfitAndLoss);
  Security security = key.m_index;
  auto entryIterator = m_securityToEntry.find(security);
  if(entryIterator == m_securityToEntry.end()) {
    beginInsertRows(QModelIndex(), static_cast<int>(m_entries.size()),
      static_cast<int>(m_entries.size()));
    std::shared_ptr<Entry> entry = std::make_shared<Entry>(security);
    m_entries.push_back(entry);
    entryIterator = m_securityToEntry.insert(make_pair(security, entry)).first;
    endInsertRows();
  }
  Entry& entry = *entryIterator->second;
  m_volume -= entry.m_volume;
  entry.m_profitAndLoss = update.m_unrealizedSecurity +
    update.m_securityInventory.m_grossProfitAndLoss -
    update.m_securityInventory.m_fees;
  entry.m_fees = update.m_securityInventory.m_fees;
  entry.m_volume = update.m_securityInventory.m_volume;
  entry.m_previousQuantity = update.m_securityInventory.m_position.m_quantity;
  int entryIndex = static_cast<int>(Find(m_entries, entry) - m_entries.begin());
  m_volume += entry.m_volume;
  m_volumeSignal(m_volume);
  Q_EMIT dataChanged(index(entryIndex, 0), index(entryIndex, COLUMN_COUNT - 1));
}

connection ProfitAndLossEntryModel::ConnectProfitAndLossSignal(
    const ProfitAndLossSignal::slot_type& slot) const {
  return m_profitAndLossSignal.connect(slot);
}

connection ProfitAndLossEntryModel::ConnectVolumeSignal(
    const VolumeSignal::slot_type& slot) const {
  return m_volumeSignal.connect(slot);
}

int ProfitAndLossEntryModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_entries.size());
}

int ProfitAndLossEntryModel::columnCount(const QModelIndex& parent) const {
  return COLUMN_COUNT;
}

QVariant ProfitAndLossEntryModel::data(const QModelIndex& index,
    int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  Entry& entry = *m_entries[index.row()];
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(index.column() == SECURITY_COLUMN) {
      return QVariant::fromValue(entry.m_security);
    } else if(index.column() == PROFIT_AND_LOSS_COLUMN) {
      return QVariant::fromValue(entry.m_profitAndLoss);
    } else if(index.column() == FEES_COLUMN) {
      return QVariant::fromValue(entry.m_fees);
    } else if(index.column() == VOLUME_COLUMN) {
      return QVariant::fromValue(entry.m_volume);
    }
  }
  return QVariant();
}

QVariant ProfitAndLossEntryModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(section == SECURITY_COLUMN) {
      return tr("Security");
    } else if(section == PROFIT_AND_LOSS_COLUMN) {
      return tr("P/L");
    } else if(section == FEES_COLUMN) {
      return tr("Fees");
    } else if(section == VOLUME_COLUMN) {
      return tr("Volume");
    }
  }
  return QVariant();
}
