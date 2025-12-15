#include "Spire/Blotter/ActivityLogModel.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

namespace {
  const auto MAX_LOWER_BOUND = 2000;
  const auto MAX_UPPER_BOUND = 4000;
}

ActivityLogModel::ActivityLogModel()
    : m_orderExecutionPublisher(nullptr) {
  m_eventHandler.emplace();
}

void ActivityLogModel::SetOrderExecutionPublisher(Ref<
    const Publisher<std::shared_ptr<Order>>> orderExecutionPublisher) {
  m_eventHandler = std::nullopt;
  m_eventHandler.emplace();
  if(!m_entries.empty()) {
    beginRemoveRows(QModelIndex(), 0, m_entries.size() - 1);
    m_entries.clear();
    endRemoveRows();
  }
  m_orderExecutionPublisher = orderExecutionPublisher.get();
  m_orderExecutionPublisher->monitor(
    m_eventHandler->get_slot<std::shared_ptr<Order>>(
      std::bind_front(&ActivityLogModel::OnOrderExecuted, this)));
}

int ActivityLogModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_entries.size());
}

int ActivityLogModel::columnCount(const QModelIndex& parent) const {
  return COLUMN_COUNT;
}

QVariant ActivityLogModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto& entry = m_entries[index.row()];
  auto& fields = entry.m_order->get_info().m_fields;
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(index.column() == TIME_COLUMN) {
      return QVariant::fromValue(entry.m_report.m_timestamp);
    } else if(index.column() == ID_COLUMN) {
      if(entry.m_report.m_id == -1) {
        return tr("N/A");
      }
      return entry.m_report.m_id;
    } else if(index.column() == SIDE_COLUMN) {
      return QVariant::fromValue(fields.m_side);
    } else if(index.column() == SECURITY_COLUMN) {
      return QVariant::fromValue(fields.m_security);
    } else if(index.column() == STATUS_COLUMN) {
      return QVariant::fromValue(entry.m_report.m_status);
    } else if(index.column() == LAST_QUANTITY_COLUMN) {
      return QVariant::fromValue(entry.m_report.m_last_quantity);
    } else if(index.column() == LAST_PRICE_COLUMN) {
      return QVariant::fromValue(entry.m_report.m_last_price);
    } else if(index.column() == LAST_MARKET_COLUMN) {
      return QString::fromStdString(entry.m_report.m_last_market);
    } else if(index.column() == LIQUIDITY_FLAG_COLUMN) {
      return QString::fromStdString(entry.m_report.m_liquidity_flag);
    } else if(index.column() == EXECUTION_FEE_COLUMN) {
      return QVariant::fromValue(entry.m_report.m_execution_fee);
    } else if(index.column() == PROCESSING_FEE_COLUMN) {
      return QVariant::fromValue(entry.m_report.m_processing_fee);
    } else if(index.column() == COMMISSION_COLUMN) {
      return QVariant::fromValue(entry.m_report.m_commission);
    } else if(index.column() == MESSAGE_COLUMN) {
      return QString::fromStdString(entry.m_report.m_text);
    }
  }
  return QVariant();
}

QVariant ActivityLogModel::headerData(
    int section, Qt::Orientation orientation, int role) const {
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(section == TIME_COLUMN) {
      return tr("Time");
    } else if(section == ID_COLUMN) {
      return tr("ID");
    } else if(section == SIDE_COLUMN) {
      return tr("Side");
    } else if(section == SECURITY_COLUMN) {
      return tr("Symbol");
    } else if(section == STATUS_COLUMN) {
      return tr("Status");
    } else if(section == LAST_QUANTITY_COLUMN) {
      return tr("Qty");
    } else if(section == LAST_PRICE_COLUMN) {
      return tr("Px");
    } else if(section == LAST_MARKET_COLUMN) {
      return tr("Mkt");
    } else if(section == LIQUIDITY_FLAG_COLUMN) {
      return tr("Flag");
    } else if(section == EXECUTION_FEE_COLUMN) {
      return tr("Exec Fee");
    } else if(section == PROCESSING_FEE_COLUMN) {
      return tr("Proc Fee");
    } else if(section == COMMISSION_COLUMN) {
      return tr("Misc Fee");
    } else if(section == MESSAGE_COLUMN) {
      return tr("Message");
    }
  }
  return QVariant();
}

void ActivityLogModel::OnExecutionReport(
    const std::shared_ptr<Order>& order, const ExecutionReport& report) {
  beginInsertRows(QModelIndex(), static_cast<int>(m_entries.size()),
    static_cast<int>(m_entries.size()));
  m_entries.push_back(UpdateEntry(order, report));
  endInsertRows();
  if(m_entries.size() >= MAX_UPPER_BOUND) {
    auto pruneCount = static_cast<int>(m_entries.size() - MAX_LOWER_BOUND);
    beginRemoveRows(QModelIndex(), 0, pruneCount - 1);
    m_entries.erase(m_entries.begin(), m_entries.begin() + pruneCount);
    endRemoveRows();
  }
}

void ActivityLogModel::OnOrderExecuted(const std::shared_ptr<Order>& order) {
  order->get_publisher().monitor(m_eventHandler->get_slot<ExecutionReport>(
    std::bind_front(&ActivityLogModel::OnExecutionReport, this, order)));
}
