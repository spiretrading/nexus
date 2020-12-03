#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  const unsigned int UPDATE_INTERVAL = 100;
}

OrderImbalanceIndicatorModel::OrderImbalanceIndicatorModel(
    Ref<UserProfile> userProfile,
    const OrderImbalanceIndicatorProperties& properties)
    : m_userProfile(userProfile.Get()),
      m_properties(properties) {
  connect(&m_updateTimer, &QTimer::timeout, this,
    &OrderImbalanceIndicatorModel::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
  InitializePublishers();
}

const OrderImbalanceIndicatorProperties& OrderImbalanceIndicatorModel::
    GetProperties() const {
  return m_properties;
}

void OrderImbalanceIndicatorModel::SetMarketFilter(MarketCode market,
    bool filter) {
  bool isCurrentlyFiltered = m_properties.IsFiltered(market);
  if(isCurrentlyFiltered == filter) {
    return;
  }
  if(filter) {
    beginResetModel();
    m_properties.m_filteredMarkets.insert(market);
    m_displayedOrderImbalances.clear();
    for(const MarketOrderImbalance& orderImbalance : m_orderImbalances) {
      if(IsDisplayed(orderImbalance)) {
        m_displayedOrderImbalances.push_back(orderImbalance);
      }
    }
    endResetModel();
  } else {
    beginResetModel();
    m_properties.m_filteredMarkets.erase(market);
    m_displayedOrderImbalances.clear();
    for(const MarketOrderImbalance& orderImbalance : m_orderImbalances) {
      if(IsDisplayed(orderImbalance)) {
        m_displayedOrderImbalances.push_back(orderImbalance);
      }
    }
    endResetModel();
  }
}

void OrderImbalanceIndicatorModel::UpdateTimeRange(
    const TimeRangeParameter& startTime, const TimeRangeParameter& endTime) {
  m_properties.m_startTime = startTime;
  m_properties.m_endTime = endTime;
  Reset();
  InitializePublishers();
}

int OrderImbalanceIndicatorModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_displayedOrderImbalances.size());
}

int OrderImbalanceIndicatorModel::columnCount(const QModelIndex& parent) const {
  return COLUMN_COUNT;
}

QVariant OrderImbalanceIndicatorModel::data(
    const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  const MarketOrderImbalance& orderImbalance =
    m_displayedOrderImbalances[index.row()];
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(index.column() == MARKET_COLUMN) {
      return QVariant::fromValue(MarketToken(orderImbalance.GetIndex()));
    } else if(index.column() == SECURITY_COLUMN) {
      return QVariant::fromValue(orderImbalance->m_security);
    } else if(index.column() == SIDE_COLUMN) {
      return QVariant::fromValue(orderImbalance->m_side);
    } else if(index.column() == SIZE_COLUMN) {
      return QVariant::fromValue(orderImbalance->m_size);
    } else if(index.column() == REFERENCE_PRICE_COLUMN) {
      return QVariant::fromValue(orderImbalance->m_referencePrice);
    } else if(index.column() == NOTIONAL_VALUE_COLUMN) {
      return QVariant::fromValue(
        orderImbalance->m_size * orderImbalance->m_referencePrice);
    } else if(index.column() == TIMESTAMP_COLUMN) {
      return QVariant::fromValue(orderImbalance->m_timestamp);
    }
  }
  return QVariant();
}

QVariant OrderImbalanceIndicatorModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(section == MARKET_COLUMN) {
      return tr("Market");
    } else if(section == SECURITY_COLUMN) {
      return tr("Security");
    } else if(section == SIDE_COLUMN) {
      return tr("Side");
    } else if(section == SIZE_COLUMN) {
      return tr("Size");
    } else if(section == REFERENCE_PRICE_COLUMN) {
      return tr("Reference Px");
    } else if(section == NOTIONAL_VALUE_COLUMN) {
      return tr("Notional Value");
    } else if(section == TIMESTAMP_COLUMN) {
      return tr("Time");
    }
  }
  return QVariant();
}

bool OrderImbalanceIndicatorModel::IsDisplayed(
    const MarketOrderImbalance& orderImbalance) const {
  return m_properties.IsDisplayed(orderImbalance.GetIndex());
}

void OrderImbalanceIndicatorModel::Reset() {
  beginResetModel();
  m_slotHandler = std::nullopt;
  m_slotHandler.emplace();
  m_displayedOrderImbalances.clear();
  m_orderImbalances.clear();
  endResetModel();
}

void OrderImbalanceIndicatorModel::InitializePublishers() {
  m_slotHandler.emplace();
  Beam::Queries::Range timeRange = m_properties.GetTimeRange(
    m_userProfile->GetServiceClients().GetTimeClient());
  for(const MarketDatabase::Entry& market :
      m_userProfile->GetMarketDatabase().GetEntries()) {
    MarketWideDataQuery orderImbalanceQuery;
    orderImbalanceQuery.SetIndex(market.m_code);
    orderImbalanceQuery.SetRange(timeRange);
    orderImbalanceQuery.SetSnapshotLimit(SnapshotLimit::Unlimited());
    m_userProfile->GetServiceClients().GetMarketDataClient().
      QueryOrderImbalances(orderImbalanceQuery,
      m_slotHandler->GetSlot<OrderImbalance>(
      std::bind(&OrderImbalanceIndicatorModel::OnOrderImbalance, this,
      market.m_code, std::placeholders::_1)));
  }
}

void OrderImbalanceIndicatorModel::OnOrderImbalance(MarketCode market,
    const OrderImbalance& orderImbalance) {
  MarketOrderImbalance marketOrderImbalance(orderImbalance, market);
  m_orderImbalances.push_back(marketOrderImbalance);
  if(IsDisplayed(marketOrderImbalance)) {
    beginInsertRows(QModelIndex(),
      static_cast<int>(m_displayedOrderImbalances.size()),
      static_cast<int>(m_displayedOrderImbalances.size()));
    m_displayedOrderImbalances.push_back(marketOrderImbalance);
    endInsertRows();
  }
}

void OrderImbalanceIndicatorModel::OnUpdateTimer() {
  HandleTasks(*m_slotHandler);
}
