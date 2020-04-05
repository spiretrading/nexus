#include "Spire/KeyBindings/CancelKeyBindingsTableModel.hpp"

using namespace Spire;

namespace {
  const auto ROW_COUNT = 13;
  const auto COLUMN_COUNT = 2;

  auto get_action(int index) {
    static auto actions = std::vector<KeyBindings::CancelAction>({
      KeyBindings::CancelAction::ALL,
      KeyBindings::CancelAction::ALL_ASKS,
      KeyBindings::CancelAction::ALL_BIDS,
      KeyBindings::CancelAction::CLOSEST_ASK,
      KeyBindings::CancelAction::CLOSEST_BID,
      KeyBindings::CancelAction::FURTHEST_BID,
      KeyBindings::CancelAction::FURTHEST_ASK,
      KeyBindings::CancelAction::MOST_RECENT,
      KeyBindings::CancelAction::MOST_RECENT_ASK,
      KeyBindings::CancelAction::MOST_RECENT_BID,
      KeyBindings::CancelAction::OLDEST,
      KeyBindings::CancelAction::OLDEST_ASK,
      KeyBindings::CancelAction::OLDEST_BID
    });
    return actions[index];
  }

  auto get_index(KeyBindings::CancelAction action) {
    static auto indexes = std::unordered_map<KeyBindings::CancelAction, int>({
      {KeyBindings::CancelAction::ALL, 0},
      {KeyBindings::CancelAction::ALL_ASKS, 1},
      {KeyBindings::CancelAction::ALL_BIDS, 2},
      {KeyBindings::CancelAction::CLOSEST_ASK, 3},
      {KeyBindings::CancelAction::CLOSEST_BID, 4},
      {KeyBindings::CancelAction::FURTHEST_BID, 5},
      {KeyBindings::CancelAction::FURTHEST_ASK, 6},
      {KeyBindings::CancelAction::MOST_RECENT, 7},
      {KeyBindings::CancelAction::MOST_RECENT_ASK, 8},
      {KeyBindings::CancelAction::MOST_RECENT_BID, 9},
      {KeyBindings::CancelAction::OLDEST, 10},
      {KeyBindings::CancelAction::OLDEST_ASK, 11},
      {KeyBindings::CancelAction::OLDEST_BID, 12}});
    return indexes[action];
  }
}

CancelKeyBindingsTableModel::CancelKeyBindingsTableModel(
    const std::vector<KeyBindings::CancelActionBinding>& bindings,
    QObject* parent)
    : QAbstractTableModel(parent) {
  for(auto i = 0; i < ROW_COUNT; ++i) {
    m_key_bindings.push_back({{}, {}, get_action(i)});
  }
  set_key_bindings(bindings);
}

void CancelKeyBindingsTableModel::set_key_bindings(
    const std::vector<KeyBindings::CancelActionBinding>& bindings) {
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
  if(role == Qt::DisplayRole) {
    if(index.column() == 0) {
      switch(index.row()) {
        case 0:
          return QObject::tr("All");
        case 1:
          return QObject::tr("All Asks");
        case 2:
          return QObject::tr("All Bids");
        case 3:
          return QObject::tr("Closest Ask");
        case 4:
          return QObject::tr("Closest Bid");
        case 5:
          return QObject::tr("Furthest Ask");
        case 6:
          return QObject::tr("Furthest Bid");
        case 7:
          return QObject::tr("Most Recent");
        case 8:
          return QObject::tr("Most Recent Ask");
        case 9:
          return QObject::tr("Most Recent Bid");
        case 10:
          return QObject::tr("Oldest");
        case 11:
          return QObject::tr("Oldest Ask");
        case 12:
          return QObject::tr("Oldest Bid");
        default:
          return QObject::tr("Invalid cancel action");
      }
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
  auto flags = QAbstractItemModel::flags(index);
  if(index.column() != 0) {
    return flags |= Qt::ItemIsEditable;
  }
  return flags;
}

QVariant CancelKeyBindingsTableModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if(section == 0) {
      return tr("Cancel Options");
    }
    if(section == 1) {
      return tr("Key Binding");
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
