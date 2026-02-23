#include "Spire/Utilities/TickerInfoModel.hpp"
#include <boost/algorithm/string/case_conv.hpp>
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

TickerInfoModel::TickerInfoModel(Ref<UserProfile> userProfile)
  : m_userProfile(userProfile.get()) {}

void TickerInfoModel::Search(const std::string& prefix) {
  if(prefix.empty()) {
    return;
  }
  auto uppercasePrefix = to_upper_copy(prefix);
  if(!m_prefixes.insert(uppercasePrefix).second) {
    return;
  }
  m_queryPromise = QtPromise([=] {
    return m_userProfile->GetClients().get_market_data_client().
      load_ticker_info_from_prefix(uppercasePrefix);
  }, LaunchPolicy::ASYNC).then(
    [=] (const std::vector<TickerInfo>& tickerInfoItems) {
      QTimer::singleShot(0, this, [=] {
        AddTickerInfoItems(tickerInfoItems);
      });
    });
}

int TickerInfoModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_tickerInfoItems.size());
}

int TickerInfoModel::columnCount(const QModelIndex& parent) const {
  return COLUMN_COUNT;
}

QVariant TickerInfoModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto& item = m_tickerInfoItems[index.row()];
  if(role == Qt::DisplayRole) {
    if(index.column() == TICKER_COLUMN) {
      return displayText(item.m_ticker);
    } else if(index.column() == NAME_COLUMN) {
      return QString::fromStdString(item.m_name);
    } else if(index.column() == SECTOR_COLUMN) {
      return QString();
    }
  }
  return QVariant();
}

QVariant TickerInfoModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(role == Qt::DisplayRole) {
    if(section == TICKER_COLUMN) {
      return tr("Ticker");
    } else if(section == NAME_COLUMN) {
      return tr("Name");
    } else if(section == SECTOR_COLUMN) {
      return tr("Sector");
    }
  }
  return QVariant();
}

void TickerInfoModel::AddTickerInfoItems(
    std::vector<TickerInfo> tickerInfoItems) {
  auto i = tickerInfoItems.begin();
  while(i != tickerInfoItems.end()) {
    auto searchIterator = find_if(m_tickerInfoItems.begin(),
      m_tickerInfoItems.end(), [&] (const auto& tickerInfo) {
        return tickerInfo.m_ticker == i->m_ticker;
      });
    if(searchIterator != m_tickerInfoItems.end()) {
      i = tickerInfoItems.erase(i);
    } else {
      ++i;
    }
  }
  if(tickerInfoItems.empty()) {
    return;
  }
  beginInsertRows(QModelIndex(), static_cast<int>(m_tickerInfoItems.size()),
    static_cast<int>(m_tickerInfoItems.size()) +
    tickerInfoItems.size() - 1);
  m_tickerInfoItems.insert(m_tickerInfoItems.end(),
    tickerInfoItems.begin(), tickerInfoItems.end());
  endInsertRows();
}
