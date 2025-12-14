#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

OrderImbalanceIndicatorModel::OrderImbalanceIndicatorModel(
    Ref<UserProfile> userProfile,
    const OrderImbalanceIndicatorProperties& properties)
    : m_userProfile(userProfile.get()),
      m_properties(properties) {
  InitializePublishers();
}

const OrderImbalanceIndicatorProperties&
    OrderImbalanceIndicatorModel::GetProperties() const {
  return m_properties;
}

void OrderImbalanceIndicatorModel::SetVenueFilter(Venue venue, bool filter) {
  if(m_properties.IsFiltered(venue) == filter) {
    return;
  }
  beginResetModel();
  if(filter) {
    m_properties.m_filteredVenues.insert(venue);
  } else {
    m_properties.m_filteredVenues.erase(venue);
  }
  m_displayedIndicies.clear();
  m_displayedOrderImbalances.clear();
  for(auto& orderImbalance : m_orderImbalances) {
    if(IsDisplayed(orderImbalance)) {
      m_displayedIndicies.insert(std::pair(
        std::pair(orderImbalance.get_index(), orderImbalance->m_security),
        static_cast<int>(m_displayedOrderImbalances.size())));
      m_displayedOrderImbalances.push_back(orderImbalance);
    }
  }
  endResetModel();
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
  auto& orderImbalance = m_displayedOrderImbalances[index.row()];
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(index.column() == VENUE_COLUMN) {
      return QVariant::fromValue(orderImbalance.get_index());
    } else if(index.column() == SECURITY_COLUMN) {
      return QVariant::fromValue(orderImbalance->m_security);
    } else if(index.column() == SIDE_COLUMN) {
      return QVariant::fromValue(orderImbalance->m_side);
    } else if(index.column() == SIZE_COLUMN) {
      return QVariant::fromValue(orderImbalance->m_size);
    } else if(index.column() == REFERENCE_PRICE_COLUMN) {
      return QVariant::fromValue(orderImbalance->m_reference_price);
    } else if(index.column() == NOTIONAL_VALUE_COLUMN) {
      return QVariant::fromValue(
        orderImbalance->m_size * orderImbalance->m_reference_price);
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
    if(section == VENUE_COLUMN) {
      return tr("Venue");
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
    const VenueOrderImbalance& orderImbalance) const {
  return m_properties.IsDisplayed(orderImbalance.get_index());
}

void OrderImbalanceIndicatorModel::Reset() {
  beginResetModel();
  m_eventHandler = std::nullopt;
  m_eventHandler.emplace();
  m_displayedIndicies.clear();
  m_imbalanceIndicies.clear();
  m_displayedOrderImbalances.clear();
  m_orderImbalances.clear();
  endResetModel();
}

void OrderImbalanceIndicatorModel::InitializePublishers() {
  m_eventHandler.emplace();
  auto timeRange = m_properties.GetTimeRange(
    m_userProfile->GetClients().get_time_client());
  for(auto& venue : DEFAULT_VENUES.get_entries()) {
    auto orderImbalanceQuery = VenueMarketDataQuery();
    orderImbalanceQuery.set_index(venue.m_venue);
    orderImbalanceQuery.set_range(timeRange);
    orderImbalanceQuery.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    m_userProfile->GetClients().get_market_data_client().query(
      orderImbalanceQuery, m_eventHandler->get_slot<OrderImbalance>(
        std::bind_front(&OrderImbalanceIndicatorModel::OnOrderImbalance, this,
          venue.m_venue)));
  }
}

void OrderImbalanceIndicatorModel::OnOrderImbalance(
    Venue venue, const OrderImbalance& orderImbalance) {
  auto venueOrderImbalance = VenueOrderImbalance(orderImbalance, venue);
  auto key = std::pair(venue, orderImbalance.m_security);
  auto i = m_imbalanceIndicies.find(key);
  auto isReplacing = false;
  if(i != m_imbalanceIndicies.end()) {
    auto& imbalance = m_orderImbalances[i->second];
    if(orderImbalance.m_timestamp - imbalance->m_timestamp <= minutes(20)) {
      imbalance = venueOrderImbalance;
      isReplacing = true;
    } else {
      m_imbalanceIndicies[key] = static_cast<int>(m_orderImbalances.size());
      m_orderImbalances.push_back(venueOrderImbalance);
    }
  } else {
    m_imbalanceIndicies.insert(
      std::pair(key, static_cast<int>(m_orderImbalances.size())));
    m_orderImbalances.push_back(venueOrderImbalance);
  }
  auto j = m_displayedIndicies.find(key);
  if(!isReplacing || j == m_displayedIndicies.end()) {
    if(IsDisplayed(venueOrderImbalance)) {
      beginInsertRows(QModelIndex(),
        static_cast<int>(m_displayedOrderImbalances.size()),
        static_cast<int>(m_displayedOrderImbalances.size()));
      m_displayedIndicies.insert(
        std::pair(key, static_cast<int>(m_displayedOrderImbalances.size())));
      m_displayedOrderImbalances.push_back(venueOrderImbalance);
      endInsertRows();
    }
  } else {
    m_displayedOrderImbalances[j->second] = venueOrderImbalance;
    dataChanged(index(j->second, 0), index(j->second, COLUMN_COUNT - 1));
  }
}
