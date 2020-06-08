#include "Spire/KeyBindings/CancelKeyBindingsTableModel.hpp"
#include <QColor>

using namespace Spire;
using Action = CancelKeyBindingsTableModel::Action;
using Binding = CancelKeyBindingsTableModel::Binding;

namespace {
  const auto ROW_COUNT = 13;
  const auto COLUMN_COUNT = 2;

  constexpr auto get_action(int index) {
    switch(index) {
      case 0:
        return Action::ALL;
      case 1:
        return Action::ALL_ASKS;
      case 2:
        return Action::ALL_BIDS;
      case 3:
        return Action::CLOSEST_ASK;
      case 4:
        return Action::CLOSEST_BID;
      case 5:
        return Action::FURTHEST_BID;
      case 6:
        return Action::FURTHEST_ASK;
      case 7:
        return Action::MOST_RECENT;
      case 8:
        return Action::MOST_RECENT_ASK;
      case 9:
        return Action::MOST_RECENT_BID;
      case 10:
        return Action::OLDEST;
      case 11:
        return Action::OLDEST_ASK;
      case 12:
        return Action::OLDEST_BID;
      default:
        return Action::ALL;
    }
  }

  constexpr auto get_index(Action action) {
    switch(action) {
      case get_action(0):
        return 0;
      case get_action(1):
        return 1;
      case get_action(2):
        return 2;
      case get_action(3):
        return 3;
      case get_action(4):
        return 4;
      case get_action(5):
        return 5;
      case get_action(6):
        return 6;
      case get_action(7):
        return 7;
      case get_action(8):
        return 8;
      case get_action(9):
        return 9;
      case get_action(10):
        return 10;
      case get_action(11):
        return 11;
      case get_action(12):
        return 12;
      default:
        return -1;
    }
  }

  constexpr auto get_action_text(int row) {
    switch(row) {
      case 0:
        return "All";
      case 1:
        return "All Asks";
      case 2:
        return "All Bids";
      case 3:
        return "Closest Ask";
      case 4:
        return "Closest Bid";
      case 5:
        return "Furthest Ask";
      case 6:
        return "Furthest Bid";
      case 7:
        return "Most Recent";
      case 8:
        return "Most Recent Ask";
      case 9:
        return "Most Recent Bid";
      case 10:
        return "Oldest";
      case 11:
        return "Oldest Ask";
      case 12:
        return "Oldest Bid";
      default:
        return "Invalid cancel action";
    }
  }
}

CancelKeyBindingsTableModel::CancelKeyBindingsTableModel(
    std::vector<Binding> bindings, QObject* parent)
    : KeyBindingsTableModel(parent) {
  m_key_bindings.reserve(ROW_COUNT);
  for(auto i = 0; i < ROW_COUNT; ++i) {
    m_key_bindings.push_back({{}, {}, get_action(i)});
  }
  set_key_bindings(std::move(bindings));
}

void CancelKeyBindingsTableModel::set_key_bindings(
    const std::vector<Binding>& bindings) {
  for(auto& new_binding : bindings) {
    for(auto& existing_binding : m_key_bindings) {
      if(existing_binding.m_sequence == new_binding.m_sequence &&
          existing_binding.m_action != new_binding.m_action) {
        existing_binding.m_sequence = QKeySequence();
        break;
      }
    }
    m_key_bindings[get_index(new_binding.m_action)].m_sequence =
      new_binding.m_sequence;
  }
}

Action CancelKeyBindingsTableModel::get_cancel_action(int row) {
  return get_action(row);
}

int CancelKeyBindingsTableModel::rowCount(const QModelIndex& parent) const {
  return ROW_COUNT;
}

int CancelKeyBindingsTableModel::columnCount(const QModelIndex& parent) const {
  return COLUMN_COUNT;
}

QVariant CancelKeyBindingsTableModel::data(const QModelIndex& index,
    int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  if(role == Qt::BackgroundRole) {
    return KeyBindingsTableModel::data(index, role);
  }
  if(role == Qt::DisplayRole) {
    if(index.column() == 0) {
      return QString::fromStdString(std::string(get_action_text(index.row())));
    } else if(index.column() == 1) {
      return m_key_bindings[index.row()].m_sequence;
    }
  }
  return QVariant();
}

Qt::ItemFlags CancelKeyBindingsTableModel::flags(
    const QModelIndex& index) const {
  if(!index.isValid()) {
    return Qt::NoItemFlags;
  }
  if(index.column() != 0) {
    return QAbstractItemModel::flags(index).setFlag(Qt::ItemIsEditable, true);
  }
  return QAbstractItemModel::flags(index).setFlag(Qt::ItemIsEnabled, false);
}

QVariant CancelKeyBindingsTableModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if(section == 0) {
      return tr("Cancel Option");
    }
    if(section == 1) {
      return tr("Key");
    }
  }
  return QVariant();
}

bool CancelKeyBindingsTableModel::setData(const QModelIndex &index,
    const QVariant &value, int role) {
  if(!index.isValid()) {
    return false;
  }
  if(role == Qt::DisplayRole && index.column() == 1) {
    for(auto i = std::size_t(0); i < m_key_bindings.size(); ++i) {
      auto& binding = m_key_bindings[i];
      if(binding.m_sequence == value.value<QKeySequence>()) {
        binding.m_sequence = QKeySequence();
        auto updated_index = this->index(i, 1);
        break;
      }
    }
    m_key_bindings[index.row()].m_sequence = value.value<QKeySequence>();
    return true;
  }
  return false;
}
