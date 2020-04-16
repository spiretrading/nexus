#include "Spire/KeyBindings/TaskKeyBindingsTableModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using Action = TaskKeyBindingsTableModel::Action;

namespace {
  const auto COLUMN_COUNT = 9;

  QVariant to_variant(const any& value) {
    if(value.type() == typeid(Quantity)) {
      return QVariant::fromValue(any_cast<Quantity>(value));
    } else if(value.type() == typeid(Region)) {
      return QVariant::fromValue(any_cast<Region>(value));
    } else if(value.type() == typeid(OrderType)) {
      return QVariant::fromValue(any_cast<OrderType>(value));
    } else if(value.type() == typeid(Security)) {
      return QVariant::fromValue(any_cast<Security>(value));
    } else if(value.type() == typeid(Side)) {
      return QVariant::fromValue(any_cast<Side>(value));
    } else if(value.type() == typeid(TimeInForce)) {
      return QVariant::fromValue(any_cast<TimeInForce>(value));
    }
    return QVariant();
  }
}

TaskKeyBindingsTableModel::TaskKeyBindingsTableModel(
  std::vector<Action> bindings, QObject* parent)
  : QAbstractTableModel(parent),
    m_key_bindings(std::move(bindings)) {}

void TaskKeyBindingsTableModel::set_key_bindings(
    const std::vector<Action>& bindings) {
  m_key_bindings = bindings;
}

int TaskKeyBindingsTableModel::rowCount(const QModelIndex& parent) const {
  return m_key_bindings.size() + 1;
}

int TaskKeyBindingsTableModel::columnCount(const QModelIndex& parent) const {
  return COLUMN_COUNT;
}

QVariant TaskKeyBindingsTableModel::data(const QModelIndex& index,
    int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  if(role == Qt::DisplayRole &&
      index.row() < static_cast<int>(m_key_bindings.size())) {
    switch(index.column()) {
      case 0:
        return QString::fromStdString(
          m_key_bindings[index.row()].m_action.m_name);
      case 1:
        {
          auto& securities =
            m_key_bindings[index.row()].m_region.GetSecurities();
          if(!securities.empty()) {
            return to_variant(*(securities.begin()));
          }
        }
        break;
      case 2:
        return to_variant(m_key_bindings[index.row()].m_region);
      case 3:
        {
          auto type = m_key_bindings[index.row()].m_action.m_type;
          if(type) {
            return to_variant(*type);
          }
        }
        break;
      case 4:
        {
          auto side = m_key_bindings[index.row()].m_action.m_side;
          if(side) {
            return to_variant(*side);
          }
        }
        break;
      case 5:
        {
          auto quantity = m_key_bindings[index.row()].m_action.m_quantity;
          if(quantity) {
            return to_variant(*quantity);
          }
        }
        break;
      case 6:
        {
          auto time_in_force =
            m_key_bindings[index.row()].m_action.m_time_in_force;
          if(time_in_force &&
              time_in_force->GetType() != TimeInForce::Type::NONE) {
            return to_variant(*time_in_force);
          }
        }
        break;
      case 7:
        return "";
      case 8:
        return m_key_bindings[index.row()].m_sequence;
      default:
        return QVariant();
    }
  }
  return QVariant();
}

Qt::ItemFlags TaskKeyBindingsTableModel::flags(
    const QModelIndex& index) const {
  return QAbstractItemModel::flags(index) |= Qt::ItemIsEditable;
}

QVariant TaskKeyBindingsTableModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case 0:
        return tr("Name");
      case 1:
        return tr("Security");
      case 2:
        return tr("Destination");
      case 3:
        return tr("Order Type");
      case 4:
        return tr("Side");
      case 5:
        return tr("Quantity");
      case 6:
        return tr("Time in Force");
      case 7:
        return tr("Custom Tags");
      case 8:
        return tr("Key");
      default:
        return QVariant();
    }
  }
  return QVariant();
}

bool TaskKeyBindingsTableModel::setData(const QModelIndex& index,
    const QVariant& value, int role) {
  if(!index.isValid()) {
    return false;
  }
  if(role == Qt::DisplayRole) {
    if(index.column() == 3) {
      if(value.isValid()) {
        m_key_bindings[index.row()].m_action.m_type = value.value<OrderType>();
      } else {
        m_key_bindings[index.row()].m_action.m_type = {};
      }
    } else if(index.column() == 4) {
      if(value.isValid()) {
        m_key_bindings[index.row()].m_action.m_side = value.value<Side>();
      } else {
        m_key_bindings[index.row()].m_action.m_side = {};
      }
    } else if(index.column() == 6) {
      if(value.isValid()) {
        m_key_bindings[index.row()].m_action.m_time_in_force =
          value.value<TimeInForce>();
      } else {
        m_key_bindings[index.row()].m_action.m_time_in_force = {};
      }
    } else if(index.column() == 8) {
      for(auto& binding : m_key_bindings) {
        if(binding.m_sequence == value.value<QKeySequence>()) {
          binding.m_sequence = QKeySequence();
          break;
        }
      }
      m_key_bindings[index.row()].m_sequence = value.value<QKeySequence>();
    }
    emit dataChanged(index, index, {role});
    return true;
  }
  return false;
}
