#include "Spire/KeyBindings/OrderTaskTableModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto to_list_model(const OrderTaskTableModel::OrderTask& order_task) {
    static auto empty_value = std::any();
    auto list_model = std::make_shared<ArrayListModel<std::any>>();
    list_model->push(empty_value);
    list_model->push(order_task.m_name);
    list_model->push(order_task.m_region);
    list_model->push(order_task.m_destination);
    list_model->push(order_task.m_order_type);
    list_model->push(order_task.m_side);
    list_model->push(order_task.m_quantity);
    list_model->push(order_task.m_time_in_force);
    list_model->push(order_task.m_key_sequence);
    return list_model;
  }
}

OrderTaskTableModel::OrderTaskTableModel(
  std::shared_ptr<ListModel<OrderTask>> source)
  : m_source(std::move(source)),
    m_source_connection(m_source->connect_operation_signal(
      std::bind_front(&OrderTaskTableModel::on_operation, this))) {}

void OrderTaskTableModel::push(
    const OrderTaskTableModel::OrderTask& order_task) {
  m_source->push(order_task);
}

void OrderTaskTableModel::move(int source, int destination) {
  if(source < 0 || source >= m_source->get_size() || destination < 0 ||
      destination >= m_source->get_size()) {
    throw std::out_of_range("The source or destination is out of range.");
  }
  m_source->move(source, destination);
}

void OrderTaskTableModel::remove(int index) {
  if(index < 0 || index >= m_source->get_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  m_source->remove(index);
}

int OrderTaskTableModel::get_row_size() const {
  return m_source->get_size() + 1;
}

int OrderTaskTableModel::get_column_size() const {
  return 9;
}

AnyRef OrderTaskTableModel::at(int row, int column) const {
  if(row < 0 || row >= get_row_size() || column < 0 ||
      column >= get_column_size()) {
    throw std::out_of_range("The row or column is out of range.");
  }
  static auto empty_value = AnyRef();
  if(row < m_source->get_size()) {
    auto& order_task = m_source->get(row);
    if(column == GRAB_HANDLE_INDEX) {
      return empty_value;
    } if(column == NAME_INDEX) {
      return order_task.m_name;
    } else if(column == REGION_INDEX) {
      return order_task.m_region;
    } else if(column == DESTINATION_INDEX) {
      return order_task.m_destination;
    } else if(column == ORDER_TYPE_INDEX) {
      return order_task.m_order_type;
    } else if(column == SIDE_INDEX) {
      return order_task.m_side;
    } else if(column == QUANTITY_INDEX) {
      return order_task.m_quantity;
    } else if(column == TIME_IN_FORCE_INDEX) {
      return order_task.m_time_in_force;
    } else if(column == KEY_INDEX) {
      return order_task.m_key_sequence;
    }
  }
  return empty_value;
}

QValidator::State OrderTaskTableModel::set(int row, int column,
    const std::any& value) {
  if(row < 0 || row >= get_row_size() || column < 0 ||
      column >= get_column_size()) {
    throw std::out_of_range("The row or column is out of range.");
  }
  if(column == GRAB_HANDLE_INDEX) {
    return QValidator::State::Invalid;
  }
  if(row < m_source->get_size()) {
    auto order_task = m_source->get(row);
    if(column == NAME_INDEX) {
      order_task.m_name = std::any_cast<QString>(value);
    } else if(column == REGION_INDEX) {
      order_task.m_region = std::any_cast<Region>(value);
    } else if(column == DESTINATION_INDEX) {
      order_task.m_destination = std::any_cast<Destination>(value);
    } else if(column == ORDER_TYPE_INDEX) {
      order_task.m_order_type = std::any_cast<OrderType>(value);
    } else if(column == SIDE_INDEX) {
      order_task.m_side = std::any_cast<Side>(value);
    } else if(column == QUANTITY_INDEX) {
      order_task.m_quantity = std::any_cast<optional<Quantity>>(value);
    } else if(column == TIME_IN_FORCE_INDEX) {
      order_task.m_time_in_force = std::any_cast<TimeInForce>(value);
    } else if(column == KEY_INDEX) {
      order_task.m_key_sequence = std::any_cast<QKeySequence>(value);
    }
    return m_source->set(row, order_task);
  }
  return QValidator::State::Invalid;
}

connection OrderTaskTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void OrderTaskTableModel::on_operation(
    const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      m_transaction.push(TableModel::AddOperation(operation.m_index,
          to_list_model(std::any_cast<const OrderTask&>(operation.m_value))));
    },
    [&] (const AnyListModel::MoveOperation& operation) {
      m_transaction.push(TableModel::MoveOperation(
        operation.m_source, operation.m_destination));
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      m_transaction.push(TableModel::RemoveOperation(operation.m_index,
        to_list_model(std::any_cast<const OrderTask&>(operation.m_value))));
    },
    [&] (const AnyListModel::UpdateOperation& operation) {
      auto& previous = std::any_cast<const OrderTask&>(operation.m_previous);
      auto& value = std::any_cast<const OrderTask&>(operation.m_value);
      auto [column, previous_field, current_field] = [&] ()
          -> std::tuple<int, std::any, std::any> {
        if(previous.m_name != value.m_name) {
          return {NAME_INDEX, previous.m_name, value.m_name};
        } else if(previous.m_region != value.m_region) {
          return {REGION_INDEX, previous.m_region, value.m_region};
        } else if(previous.m_destination != value.m_destination) {
          return {DESTINATION_INDEX, previous.m_destination,
            value.m_destination};
        } else if(previous.m_order_type != value.m_order_type) {
          return {ORDER_TYPE_INDEX, previous.m_order_type, value.m_order_type};
        } else if(previous.m_side != value.m_side) {
          return {SIDE_INDEX, previous.m_side, value.m_side};
        } else if(previous.m_quantity != value.m_quantity) {
          return {QUANTITY_INDEX, previous.m_quantity, value.m_quantity};
        } else if(previous.m_time_in_force != value.m_time_in_force) {
          return {TIME_IN_FORCE_INDEX, previous.m_time_in_force,
            value.m_time_in_force};
        } else if(previous.m_key_sequence != value.m_key_sequence) {
          return {KEY_INDEX, previous.m_key_sequence, value.m_key_sequence};
        }
        static auto empty_value = std::any();
        return {GRAB_HANDLE_INDEX, empty_value, empty_value};
      }();
      m_transaction.push(TableModel::UpdateOperation(operation.m_index,
        column, previous_field, current_field));
    });
}
