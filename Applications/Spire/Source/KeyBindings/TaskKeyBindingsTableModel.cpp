#include "Spire/KeyBindings/TaskKeyBindingsTableModel.hpp"
#include "Nexus/Definitions/Quantity.hpp"
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
  if(role == Qt::BackgroundRole) {
    if(m_highlighted_row && index.row() == m_highlighted_row) {
      return QColor("#FFF1F1");
    }
    return QVariant::fromValue<QColor>(Qt::white);
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
        {
          auto type = m_key_bindings[index.row()].m_action.m_type;
          if(type) {
            return to_variant(*type);
          }
        }
        break;
      case Columns::SIDE:
        {
          auto side = m_key_bindings[index.row()].m_action.m_side;
          if(side) {
            return to_variant(*side);
          }
        }
        break;
      case Columns::QUANTITY:
        {
          auto quantity = m_key_bindings[index.row()].m_action.m_quantity;
          if(quantity) {
            return to_variant(*quantity);
          }
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
        return tr("Custom Tags");
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
  }
  if(role == Qt::DisplayRole && value.isValid()) {
    switch(static_cast<Columns>(index.column())) {
      case Columns::NAME:
        if(index.row() == m_key_bindings.size() &&
            value.value<QString>().isEmpty()) {
          return false;
        }
        emit dataChanged(index, index, {role});
        if(index.row() == m_key_bindings.size()) {
          beginInsertRows(QModelIndex(), index.row(), index.row());
          m_key_bindings.push_back({});
          endInsertRows();
        }
        m_key_bindings[index.row()].m_action.m_name =
          value.value<QString>().toStdString();
        return true;
      case Columns::SECURITY:
        return false;
      case Columns::DESTINATION:
        return false;
      case Columns::ORDER_TYPE:
        emit dataChanged(index, index, {role});
        if(value.isValid()) {
          if(index.row() == m_key_bindings.size()) {
            beginInsertRows(QModelIndex(), index.row(), index.row());
            m_key_bindings.push_back({});
            endInsertRows();
          }
          m_key_bindings[index.row()].m_action.m_type =
            value.value<OrderType>();
        } else {
          m_key_bindings[index.row()].m_action.m_type = {};
        }
        return true;
      case Columns::SIDE:
        emit dataChanged(index, index, {role});
        if(value.isValid()) {
          if(index.row() == m_key_bindings.size()) {
            beginInsertRows(QModelIndex(), index.row(), index.row());
            m_key_bindings.push_back({});
            endInsertRows();
          }
          m_key_bindings[index.row()].m_action.m_side = value.value<Side>();
        } else {
          m_key_bindings[index.row()].m_action.m_side = {};
        }
        return true;
      case Columns::QUANTITY:
        emit dataChanged(index, index, {role});
        if(value.isValid() && value.value<Quantity>() > 0) {
          if(index.row() == m_key_bindings.size()) {
            beginInsertRows(QModelIndex(), index.row(), index.row());
            m_key_bindings.push_back({});
            endInsertRows();
          }
          m_key_bindings[index.row()].m_action.m_quantity =
            value.value<Quantity>();
        } else {
          m_key_bindings[index.row()].m_action.m_quantity = {};
        }
        return true;
      case Columns::TIME_IN_FORCE:
        emit dataChanged(index, index, {role});
        if(value.isValid()) {
          if(index.row() == m_key_bindings.size()) {
            beginInsertRows(QModelIndex(), index.row(), index.row());
            m_key_bindings.push_back({});
            endInsertRows();
          }
          m_key_bindings[index.row()].m_action.m_time_in_force =
            value.value<TimeInForce>();
        } else {
          m_key_bindings[index.row()].m_action.m_time_in_force = {};
        }
        return true;
      case Columns::CUSTOM_TAGS:
      case Columns::KEY_BINDING:
        if(index.row() == m_key_bindings.size()) {
          if(!value.value<QKeySequence>().isEmpty()) {
            beginInsertRows(QModelIndex(), index.row(), index.row());
            m_key_bindings.push_back({});
            endInsertRows();
          } else {
            return false;
          }
        }
        for(auto& binding : m_key_bindings) {
          if(binding.m_sequence == value.value<QKeySequence>()) {
            binding.m_sequence = QKeySequence();
            break;
          }
        }
        m_key_bindings[index.row()].m_sequence = value.value<QKeySequence>();
        emit dataChanged(index, index, {role});
        return true;
      default:
        return false;
    }
  }
  return false;
}
