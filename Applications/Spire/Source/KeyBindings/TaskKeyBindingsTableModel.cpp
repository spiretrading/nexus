#include "Spire/KeyBindings/TaskKeyBindingsTableModel.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using Action = TaskKeyBindingsTableModel::Action;
using Columns = TaskKeyBindingsTableModel::Columns;

namespace {
  const auto COLUMN_COUNT = 9;

  QVariant to_variant(const any& value) {
    auto& type = value.type();
    if(type == typeid(Quantity)) {
      return QVariant::fromValue(any_cast<Quantity>(value));
    } else if(type == typeid(Region)) {
      return QVariant::fromValue(any_cast<Region>(value));
    } else if(type == typeid(OrderType)) {
      return QVariant::fromValue(any_cast<OrderType>(value));
    } else if(type == typeid(Security)) {
      return QVariant::fromValue(any_cast<Security>(value));
    } else if(type == typeid(Side)) {
      return QVariant::fromValue(any_cast<Side>(value));
    } else if(type == typeid(TimeInForce)) {
      return QVariant::fromValue(any_cast<TimeInForce>(value));
    }
    return QVariant();
  }

  bool is_same_value(const QVariant& value, const QModelIndex& index) {
    auto data = index.data();
    switch(static_cast<TaskKeyBindingsTableModel::Columns>(index.column())) {
      case Columns::NAME:
        return value.value<QString>() == data.value<QString>();
      case Columns::SECURITY:
        return value.value<Security>() == data.value<Security>();
      case Columns::DESTINATION:
        return value.value<Region>() == data.value<Region>() &&
          value.value<Region>().GetName() == data.value<Region>().GetName();
      case Columns::ORDER_TYPE:
        return value.value<OrderType>() == data.value<OrderType>();
      case Columns::SIDE:
        return value.value<Side>() == data.value<Side>();
      case Columns::QUANTITY:
        return value.value<Quantity>() == data.value<Quantity>();
      case Columns::TIME_IN_FORCE:
        return value.value<TimeInForce>() == data.value<TimeInForce>();
      case Columns::CUSTOM_TAGS:
        return false;
      case Columns::KEY_BINDING:
        return value.value<QKeySequence>() == data.value<QKeySequence>();
      default:
        return false;
    }
  }
}

TaskKeyBindingsTableModel::TaskKeyBindingsTableModel(
  std::vector<Action> bindings, QObject* parent)
  : KeyBindingsTableModel(parent),
    m_key_bindings(std::move(bindings)) {}

void TaskKeyBindingsTableModel::set_key_bindings(
    const std::vector<Action>& bindings) {
  m_key_bindings = bindings;
}

connection TaskKeyBindingsTableModel::connect_item_modified_signal(
    const ItemModifiedSignal::slot_type& slot) const {
  return m_modified_signal.connect(slot);
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
    return KeyBindingsTableModel::data(index, role);
  }
  if(role == Qt::DisplayRole &&
      static_cast<std::size_t>(index.row()) < m_key_bindings.size()) {
    auto binding = m_key_bindings[index.row()];
    switch(static_cast<Columns>(index.column())) {
      case Columns::NAME:
        return QString::fromStdString(binding.m_action.m_name);
      case Columns::SECURITY:
        {
          const auto& securities = binding.m_region.GetSecurities();
          if(!securities.empty()) {
            return to_variant(*(securities.cbegin()));
          }
        }
        break;
      case Columns::DESTINATION:
        return to_variant(binding.m_region);
      case Columns::ORDER_TYPE:
        if(auto type = binding.m_action.m_type) {
          return to_variant(*type);
        }
        break;
      case Columns::SIDE:
        if(auto side = binding.m_action.m_side) {
          return to_variant(*side);
        }
        break;
      case Columns::QUANTITY:
        if(auto quantity = binding.m_action.m_quantity) {
          return to_variant(*quantity);
        }
        break;
      case Columns::TIME_IN_FORCE:
        {
          auto time_in_force = binding.m_action.m_time_in_force;
          if(time_in_force &&
              time_in_force->GetType() != TimeInForce::Type::NONE) {
            return to_variant(*time_in_force);
          }
        }
        break;
      case Columns::CUSTOM_TAGS:
        return "";
      case Columns::KEY_BINDING:
        return binding.m_sequence;
      default:
        return QVariant();
    }
  }
  return QVariant();
}

Qt::ItemFlags TaskKeyBindingsTableModel::flags(
    const QModelIndex& index) const {
  if(static_cast<Columns>(index.column()) == Columns::CUSTOM_TAGS) {
    return QAbstractItemModel::flags(index).setFlag(Qt::ItemIsEnabled, false);
  }
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
  if(row < 0 || static_cast<std::size_t>(row) >= m_key_bindings.size()) {
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
  if(role == Qt::DisplayRole) {
    if(is_same_value(value, index)) {
      return false;
    }
    insert_row_if_empty(index);
    auto& binding = m_key_bindings[index.row()];
    switch(static_cast<Columns>(index.column())) {
      case Columns::NAME:
        binding.m_action.m_name = value.value<QString>().toStdString();
        m_modified_signal(index);
        break;
      case Columns::SECURITY:
        binding.m_region = [&] () -> Region {
          auto security = value.value<Security>();
          if(security != Security()) {
            auto new_binding = Region(security);
            new_binding.SetName(binding.m_region.GetName());
            return new_binding;
          }
          if(!binding.m_region.GetName().empty()) {
            return binding.m_region;
          }
          return Region();
        }();
        m_modified_signal(index);
        break;
      case Columns::DESTINATION:
        {
          auto name = value.value<Region>().GetName();
          if(name.empty()) {
            if(binding.m_region.GetSecurities().empty()) {
              binding.m_region = Region();
            } else {
              binding.m_region = *(binding.m_region.GetSecurities().begin());
            }
          } else {
            if(binding.m_region.GetSecurities().empty()) {
              binding.m_region = Security();
            }
            binding.m_region.SetName(name);
          }
        }
        m_modified_signal(index);
        break;
      case Columns::ORDER_TYPE:
        binding.m_action.m_type =
          [&] () -> boost::optional<OrderType> {
            auto type = value.value<OrderType>();
            if(type != OrderType::NONE) {
              return type;
            }
            return boost::none;
          }();
        m_modified_signal(index);
        break;
      case Columns::SIDE:
        binding.m_action.m_side =
          [&] () -> boost::optional<Side> {
            auto side = value.value<Side>();
            if(side != Side::NONE) {
              return side;
            }
            return boost::none;
          }();
        m_modified_signal(index);
        break;
      case Columns::QUANTITY:
        binding.m_action.m_quantity =
          [&] () -> boost::optional<Quantity> {
            auto quantity = value.value<Quantity>();
            if(quantity > 0) {
              return quantity;
            }
            return boost::none;
          }();
        m_modified_signal(index);
        break;
      case Columns::TIME_IN_FORCE:
        binding.m_action.m_time_in_force =
          [&] () -> boost::optional<TimeInForce> {
            auto time = value.value<TimeInForce>();
            if(time.GetType() != TimeInForce::Type::NONE) {
              return time;
            }
            return boost::none;
          }();
        m_modified_signal(index);
        break;
      case Columns::CUSTOM_TAGS:
        break;
      case Columns::KEY_BINDING:
        if(!value.value<QKeySequence>().isEmpty()) {
          auto binding_index = index.row();
          for(auto i = 0; i < rowCount(index); ++i) {
            auto current_index = this->index(i, index.column());
            auto sequence = current_index.data().value<QKeySequence>();
            if(sequence == value.value<QKeySequence>()) {
              m_key_bindings[i].m_sequence = QKeySequence();
              m_modified_signal(current_index);
              if(is_row_empty(i)) {
                removeRow(i);
                if(i < binding_index) {
                  --binding_index;
                }
              }
              m_key_bindings[binding_index].m_sequence =
                value.value<QKeySequence>();
              m_modified_signal(this->index(binding_index, 8));
              return true;
            }
          }
        }
        binding.m_sequence = value.value<QKeySequence>();
        m_modified_signal(index);
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

bool TaskKeyBindingsTableModel::is_row_empty(int row) const {
  auto& binding = m_key_bindings[row];
  auto& action = binding.m_action;
  return action.m_name.empty() &&
    !action.m_quantity.is_initialized() &&
    !action.m_side.is_initialized() &&
    action.m_tags.empty() &&
    !action.m_time_in_force.is_initialized() &&
    !action.m_type.is_initialized() &&
    binding.m_region == Region() &&
    binding.m_sequence.isEmpty();
}
