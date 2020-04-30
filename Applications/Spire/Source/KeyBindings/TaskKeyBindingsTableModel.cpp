#include "Spire/KeyBindings/TaskKeyBindingsTableModel.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using Action = TaskKeyBindingsTableModel::Action;
using Columns = TaskKeyBindingsTableModel::Columns;

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

  bool is_same_value(const QVariant& value, const QModelIndex& index) {
    switch(static_cast<TaskKeyBindingsTableModel::Columns>(index.column())) {
      case Columns::NAME:
        return value.value<QString>() == index.data().value<QString>();
      case Columns::SECURITY:
        return value.value<Security>() == index.data().value<Security>();
      case Columns::DESTINATION:
        return value.value<Region>() == index.data().value<Region>();
      case Columns::ORDER_TYPE:
        return value.value<OrderType>() == index.data().value<OrderType>();
      case Columns::SIDE:
        return value.value<Side>() == index.data().value<Side>();
      case Columns::QUANTITY:
        return value.value<Quantity>() == index.data().value<Quantity>();
      case Columns::TIME_IN_FORCE:
        return value.value<TimeInForce>() == index.data().value<TimeInForce>();
      case Columns::CUSTOM_TAGS:
        return false;
      case Columns::KEY_BINDING:
        return value.value<QKeySequence>() ==
          index.data().value<QKeySequence>();
      default:
        return false;
    }
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
  if(role == Qt::BackgroundRole) {
    if(m_highlighted_row && index.row() == m_highlighted_row) {
      return QColor("#FFF1F1");
    }
    return QColor(Qt::white);
  }
  if(role == Qt::DisplayRole &&
      index.row() < static_cast<int>(m_key_bindings.size())) {
    switch(static_cast<Columns>(index.column())) {
      case Columns::NAME:
        return QString::fromStdString(
          m_key_bindings[index.row()].m_action.m_name);
      case Columns::SECURITY:
        {
          auto& securities =
            m_key_bindings[index.row()].m_region.GetSecurities();
          if(!securities.empty()) {
            return to_variant(*(securities.begin()));
          }
        }
        break;
      case Columns::DESTINATION:
        return to_variant(m_key_bindings[index.row()].m_region);
      case Columns::ORDER_TYPE:
        if(auto type = m_key_bindings[index.row()].m_action.m_type) {
          return to_variant(*type);
        }
        break;
      case Columns::SIDE:
        if(auto side = m_key_bindings[index.row()].m_action.m_side) {
          return to_variant(*side);
        }
        break;
      case Columns::QUANTITY:
        if(auto quantity = m_key_bindings[index.row()].m_action.m_quantity) {
          return to_variant(*quantity);
        }
        break;
      case Columns::TIME_IN_FORCE:
        {
          auto time_in_force =
            m_key_bindings[index.row()].m_action.m_time_in_force;
          if(time_in_force &&
              time_in_force->GetType() != TimeInForce::Type::NONE) {
            return to_variant(*time_in_force);
          }
        }
        break;
      case Columns::CUSTOM_TAGS:
        return "";
      case Columns::KEY_BINDING:
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
    switch(static_cast<Columns>(section)) {
      case Columns::NAME:
        return tr("Name");
      case Columns::SECURITY:
        return tr("Security");
      case Columns::DESTINATION:
        return tr("Destination");
      case Columns::ORDER_TYPE:
        return tr("Order Type");
      case Columns::SIDE:
        return tr("Side");
      case Columns::QUANTITY:
        return tr("Quantity");
      case Columns::TIME_IN_FORCE:
        return tr("Time in Force");
      case Columns::CUSTOM_TAGS:
        return tr("Custom Tag");
      case Columns::KEY_BINDING:
        return tr("Key");
      default:
        return QVariant();
    }
  }
  return QVariant();
}

bool TaskKeyBindingsTableModel::removeRows(int row, int count,
    const QModelIndex &parent) {
  if(row >= static_cast<int>(m_key_bindings.size())) {
    return false;
  }
  beginRemoveRows(parent, row, row + count);
  m_key_bindings.erase(m_key_bindings.begin() + row);
  endRemoveRows();
  return true;
}

bool TaskKeyBindingsTableModel::setData(const QModelIndex& index,
    const QVariant& value, int role) {
  if(!index.isValid()) {
    return false;
  }
  if(role == Qt::BackgroundRole) {
    if(value.isValid()) {
      m_highlighted_row = index.row();
    } else {
      m_highlighted_row.reset();
    }
    emit dataChanged(index, index, {role});
    return true;
  }
  if(role == Qt::DisplayRole) {
    if(is_same_value(value, index)) {
      return false;
    }
    emit dataChanged(index, index, {role});
    insert_row_if_empty(index);
    auto column = static_cast<Columns>(index.column());
    switch(static_cast<Columns>(index.column())) {
      case Columns::NAME:
        m_key_bindings[index.row()].m_action.m_name =
          value.value<QString>().toStdString();
      case Columns::SECURITY:
        m_key_bindings[index.row()].m_region = value.value<Security>();
      case Columns::DESTINATION:
        return false;
      case Columns::ORDER_TYPE:
        m_key_bindings[index.row()].m_action.m_type = value.value<OrderType>();
        break;
      case Columns::SIDE:
        m_key_bindings[index.row()].m_action.m_side = value.value<Side>();
        break;
      case Columns::QUANTITY:
        m_key_bindings[index.row()].m_action.m_quantity =
          value.value<Quantity>();
        break;
      case Columns::TIME_IN_FORCE:
        m_key_bindings[index.row()].m_action.m_time_in_force =
          value.value<TimeInForce>();
        break;
      case Columns::CUSTOM_TAGS:
        break;
      case Columns::KEY_BINDING:
        for(auto& binding : m_key_bindings) {
          if(binding.m_sequence == value.value<QKeySequence>()) {
            binding.m_sequence = QKeySequence();
            break;
          }
        }
        m_key_bindings[index.row()].m_sequence = value.value<QKeySequence>();
        break;
      default:
        break;
    }
    if(is_row_empty(index.row())) {
      removeRow(index.row());
    }
    return true;
  }
  return false;
}

void TaskKeyBindingsTableModel::insert_row_if_empty(const QModelIndex& index) {
  if(index.row() == m_key_bindings.size()) {
    beginInsertRows(QModelIndex(), index.row(), index.row());
    m_key_bindings.push_back({});
    endInsertRows();
  }
}

bool TaskKeyBindingsTableModel::is_row_empty(int row) {
  auto& binding = m_key_bindings[row];
  return binding.m_action.m_name.empty() &&
      !binding.m_action.m_quantity.is_initialized() &&
      !binding.m_action.m_side.is_initialized() &&
      binding.m_action.m_tags.empty() &&
      !binding.m_action.m_time_in_force.is_initialized() &&
      !binding.m_action.m_type.is_initialized() &&
      binding.m_region == Region() &&
      binding.m_sequence.isEmpty();
}
