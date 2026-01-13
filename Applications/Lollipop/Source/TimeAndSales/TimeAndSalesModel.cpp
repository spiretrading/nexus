#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include <QCoreApplication>
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

TimeAndSalesModel::TimeAndSalesModel(Ref<UserProfile> userProfile,
    const TimeAndSalesProperties& properties, const Security& security)
    : m_userProfile(userProfile.get()),
      m_properties(properties) {
  if(security == Security()) {
    return;
  }
  auto marketStartOfDay = utc_start_of_day(security.get_venue(),
    m_userProfile->GetClients().get_time_client().get_time(),
    m_userProfile->GetVenueDatabase(), m_userProfile->GetTimeZoneDatabase());
  auto query = SecurityMarketDataQuery();
  query.set_index(security);
  query.set_range(marketStartOfDay, Beam::Sequence::LAST);
  query.set_snapshot_limit(SnapshotLimit::Type::TAIL, 50);
  query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  m_userProfile->GetClients().get_market_data_client().query(
    query, m_eventHandler.get_slot<TimeAndSale>(
      std::bind_front(&TimeAndSalesModel::OnTimeAndSale, this)));
  auto bboQuery = make_current_query(security);
  bboQuery.set_interruption_policy(InterruptionPolicy::IGNORE_CONTINUE);
  m_userProfile->GetClients().get_market_data_client().query(
    bboQuery, m_eventHandler.get_slot<BboQuote>(
      std::bind_front(&TimeAndSalesModel::OnBbo, this)));
}

void TimeAndSalesModel::SetProperties(
    const TimeAndSalesProperties& properties) {
  m_properties = properties;
  if(!m_entries.empty()) {
    Q_EMIT dataChanged(index(0, 0),
      index(static_cast<int>(m_entries.size()) - 1,
      TimeAndSalesProperties::COLUMN_COUNT - 1));
  }
  beginResetModel();
  endResetModel();
}

int TimeAndSalesModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_entries.size());
}

int TimeAndSalesModel::columnCount(const QModelIndex& parent) const {
  return TimeAndSalesProperties::COLUMN_COUNT;
}

QVariant TimeAndSalesModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto& entry = m_entries[m_entries.size() - index.row() - 1];
  if(role == Qt::TextAlignmentRole) {
    if(index.column() == MARKET_COLUMN || index.column() == CONDITION_COLUMN) {
      return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  } else if(role == Qt::BackgroundRole) {
    return m_properties.GetPriceRangeBackgroundColor()[entry.second];
  } else if(role == Qt::ForegroundRole) {
    return m_properties.GetPriceRangeForegroundColor()[entry.second];
  } else if(role == Qt::DisplayRole) {
    auto& timeAndSale = entry.first;
    if(index.column() == TIME_COLUMN) {
      return QVariant::fromValue(timeAndSale.m_timestamp);
    } else if(index.column() == PRICE_COLUMN) {
      return QVariant::fromValue(timeAndSale.m_price);
    } else if(index.column() == SIZE_COLUMN) {
      return QVariant::fromValue(timeAndSale.m_size);
    } else if(index.column() == MARKET_COLUMN) {
      return QString::fromStdString(timeAndSale.m_market_center);
    } else if(index.column() == CONDITION_COLUMN) {
      return QString::fromStdString(timeAndSale.m_condition.m_code);
    }
  }
  return QVariant();
}

QVariant TimeAndSalesModel::headerData(
    int section, Qt::Orientation orientation, int role) const {
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(section == TIME_COLUMN) {
      return tr("Time");
    } else if(section == PRICE_COLUMN) {
      return tr("Px");
    } else if(section == SIZE_COLUMN) {
      return tr("Qty");
    } else if(section == MARKET_COLUMN) {
      return tr("Mkt");
    } else if(section == CONDITION_COLUMN) {
      return tr("Cond");
    }
  }
  return QVariant();
}

void TimeAndSalesModel::OnBbo(const BboQuote& bbo) {
  m_bbo = bbo;
}

void TimeAndSalesModel::OnTimeAndSale(const TimeAndSale& timeAndSale) {
  auto priceRange = [&] {
    if(m_bbo.m_ask.m_side == Side::NONE) {
      return UNKNOWN;
    } else if(timeAndSale.m_price == m_bbo.m_bid.m_price) {
      return AT_BID;
    } else if(timeAndSale.m_price < m_bbo.m_bid.m_price) {
      return BELOW_BID;
    } else if(timeAndSale.m_price == m_bbo.m_ask.m_price) {
      return AT_ASK;
    } else if(timeAndSale.m_price > m_bbo.m_ask.m_price) {
      return ABOVE_ASK;
    }
    return INSIDE;
  }();
  beginInsertRows(QModelIndex(), 0, 0);
  m_entries.push_back(std::pair(timeAndSale, priceRange));
  endInsertRows();
}
