#include "Spire/KeyBindings/TaskKeyBindingsTableModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Spire;
using Action = TaskKeyBindingsTableModel::Action;

namespace {
  const auto COLUMN_COUNT = 9;
  const auto MIN_ROW_COUNT = 10;
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
  return max(static_cast<int>(m_key_bindings.size()) + 1, MIN_ROW_COUNT);
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
      case 2:
      case 3:
      case 4:
        {
          auto side = m_key_bindings[index.row()].m_action.m_side;
          if(side) {
            return tr(ToString(*side).c_str());
          }
        }
      case 5:
        {
          auto quantity = m_key_bindings[index.row()].m_action.m_quantity;
          if(quantity) {
            return QVariant::fromValue<Nexus::Quantity>(*quantity);
          }
        }
      case 6:
      case 7:
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
  if(role == Qt::DisplayRole && index.column() == 8) {
    for(auto& binding : m_key_bindings) {
      if(binding.m_sequence == value.value<QKeySequence>()) {
        binding.m_sequence = QKeySequence();
        break;
      }
    }
    m_key_bindings[index.row()].m_sequence = value.value<QKeySequence>();
    emit dataChanged(index, index, {role});
    return true;
  }
  return false;
}
