#include "Spire/Blotter/OrderLogModel.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::LegacyUI;

OrderLogModel::OrderEntry::OrderEntry(const Order* order)
  : m_order(order),
    m_status(OrderStatus::PENDING_NEW) {}

OrderLogModel::OrderLogModel(const OrderLogProperties& properties)
    : m_properties(properties),
      m_orderExecutionPublisher(nullptr) {
  m_eventHandler.emplace();
}

const OrderLogProperties& OrderLogModel::GetProperties() const {
  return m_properties;
}

void OrderLogModel::SetProperties(const OrderLogProperties& properties) {
  m_properties = properties;
  Q_EMIT dataChanged(index(0, 0),
    index(m_entries.size() - 1, COLUMN_COUNT - 1));
}

const OrderLogModel::OrderEntry& OrderLogModel::GetEntry(
    const QModelIndex& index) const {
  return m_entries[index.row()];
}

void OrderLogModel::SetOrderExecutionPublisher(
    Ref<const OrderExecutionPublisher> orderExecutionPublisher) {
  if(!m_entries.empty()) {
    beginRemoveRows(QModelIndex(), 0, m_entries.size() - 1);
    auto entries = std::vector<OrderEntry>();
    entries.swap(m_entries);
    endRemoveRows();
    for(auto& entry : entries) {
      m_orderRemovedSignal(entry);
    }
  }
  m_eventHandler = std::nullopt;
  m_eventHandler.emplace();
  m_orderExecutionPublisher = orderExecutionPublisher.Get();
  m_orderExecutionPublisher->Monitor(m_eventHandler->get_slot<const Order*>(
    std::bind_front(&OrderLogModel::OnOrderExecuted, this)));
}

connection OrderLogModel::ConnectOrderAddedSignal(
    const OrderAddedSignal::slot_function_type& slot) const {
  return m_orderAddedSignal.connect(slot);
}

connection OrderLogModel::ConnectOrderRemovedSignal(
    const OrderRemovedSignal::slot_function_type& slot) const {
  return m_orderRemovedSignal.connect(slot);
}

int OrderLogModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_entries.size());
}

int OrderLogModel::columnCount(const QModelIndex& parent) const {
  return COLUMN_COUNT;
}

QVariant OrderLogModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto& entry = m_entries[index.row()];
  auto& fields = entry.m_order->GetInfo().m_fields;
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(index.column() == TIME_COLUMN) {
      return QVariant::fromValue(entry.m_order->GetInfo().m_timestamp);
    } else if(index.column() == ID_COLUMN) {
      return entry.m_order->GetInfo().m_orderId;
    } else if(index.column() == STATUS_COLUMN) {
      return QVariant::fromValue(entry.m_status);
    } else if(index.column() == SECURITY_COLUMN) {
      return QVariant::fromValue(fields.m_security);
    } else if(index.column() == CURRENCY_COLUMN) {
      return QVariant::fromValue(fields.m_currency);
    } else if(index.column() == ORDER_TYPE_COLUMN) {
      return QVariant::fromValue(fields.m_type);
    } else if(index.column() == SIDE_COLUMN) {
      return QVariant::fromValue(fields.m_side);
    } else if(index.column() == DESTINATION_COLUMN) {
      return QString::fromStdString(fields.m_destination);
    } else if(index.column() == QUANTITY_COLUMN) {
      return QVariant::fromValue(fields.m_quantity);
    } else if(index.column() == PRICE_COLUMN) {
      if(fields.m_type == OrderType::LIMIT ||
          fields.m_type == OrderType::PEGGED && fields.m_price != Money::ZERO) {
        return QVariant::fromValue(fields.m_price);
      } else {
        return tr("N/A");
      }
    } else if(index.column() == TIME_IN_FORCE_COLUMN) {
      return QVariant::fromValue(fields.m_timeInForce);
    }
  }
  return QVariant();
}

QVariant OrderLogModel::headerData(int section, Qt::Orientation orientation,
    int role) const {
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(section == TIME_COLUMN) {
      return tr("Time");
    } else if(section == ID_COLUMN) {
      return tr("ID");
    } else if(section == STATUS_COLUMN) {
      return tr("Status");
    } else if(section == SECURITY_COLUMN) {
      return tr("Symbol");
    } else if(section == CURRENCY_COLUMN) {
      return tr("Currency");
    } else if(section == ORDER_TYPE_COLUMN) {
      return tr("Type");
    } else if(section == SIDE_COLUMN) {
      return tr("Side");
    } else if(section == DESTINATION_COLUMN) {
      return tr("Dest.");
    } else if(section == QUANTITY_COLUMN) {
      return tr("Qty.");
    } else if(section == PRICE_COLUMN) {
      return tr("Px");
    } else if(section == TIME_IN_FORCE_COLUMN) {
      return tr("TIF");
    }
  }
  return QVariant();
}

void OrderLogModel::OnOrderExecuted(const Order* order) {
  auto index = m_entries.size();
  beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size());
  order->GetPublisher().Monitor(m_eventHandler->get_slot<ExecutionReport>(
    std::bind_front(&OrderLogModel::OnExecutionReport, this, index)));
  auto entry = OrderEntry(order);
  m_entries.push_back(entry);
  endInsertRows();
  m_orderAddedSignal(entry);
}

void OrderLogModel::OnExecutionReport(
    std::size_t entryIndex, const ExecutionReport& report) {
  m_entries[entryIndex].m_status = report.m_status;
  Q_EMIT dataChanged(index(entryIndex, STATUS_COLUMN),
    index(entryIndex, STATUS_COLUMN));
}
