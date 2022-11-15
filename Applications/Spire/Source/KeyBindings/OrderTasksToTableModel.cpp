#include "Spire/KeyBindings/OrderTasksToTableModel.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto to_list(const OrderTask& order_task) {
    auto list_model = std::make_shared<ArrayListModel<std::any>>();
    list_model->push(order_task.m_name);
    list_model->push(order_task.m_region);
    list_model->push(order_task.m_destination);
    list_model->push(order_task.m_order_type);
    list_model->push(order_task.m_side);
    list_model->push(order_task.m_quantity);
    list_model->push(order_task.m_time_in_force);
    list_model->push(order_task.m_key);
    return list_model;
  }
}

OrderTasksToTableModel::OrderTasksToTableModel(
  std::shared_ptr<ListModel<OrderTask>> source)
  : m_source(std::move(source)),
    m_source_connection(m_source->connect_operation_signal(
      std::bind_front(&OrderTasksToTableModel::on_operation, this))) {}

int OrderTasksToTableModel::get_row_size() const {
  return m_source->get_size();
}

int OrderTasksToTableModel::get_column_size() const {
  return COLUMN_SIZE;
}

AnyRef OrderTasksToTableModel::at(int row, int column) const {
  if(row < 0 || row >= get_row_size() || column < 0 ||
      column >= get_column_size()) {
    throw std::out_of_range("The row or column is out of range.");
  }
  return extract_field(m_source->get(row), static_cast<Column>(column));
}

QValidator::State OrderTasksToTableModel::set(int row, int column,
    const std::any& value) {
  if(row < 0 || row >= get_row_size() || column < 0 ||
      column >= get_column_size()) {
    throw std::out_of_range("The row or column is out of range.");
  }
  auto column_index = static_cast<Column>(column);
  auto order_task = m_source->get(row);
  auto previous = to_any(extract_field(order_task, column_index));
  if(column_index == Column::NAME) {
    order_task.m_name = std::any_cast<QString>(value);
  } else if(column_index == Column::REGION) {
    order_task.m_region = std::any_cast<Region>(value);
  } else if(column_index == Column::DESTINATION) {
    order_task.m_destination = std::any_cast<Destination>(value);
  } else if(column_index == Column::ORDER_TYPE) {
    order_task.m_order_type = std::any_cast<OrderType>(value);
  } else if(column_index == Column::SIDE) {
    order_task.m_side = std::any_cast<Side>(value);
  } else if(column_index == Column::QUANTITY) {
    order_task.m_quantity = std::any_cast<optional<Quantity>>(value);
  } else if(column_index == Column::TIME_IN_FORCE) {
    order_task.m_time_in_force = std::any_cast<TimeInForce>(value);
  } else if(column_index == Column::KEY) {
    order_task.m_key = std::any_cast<QKeySequence>(value);
  }
  auto blocker = shared_connection_block(m_source_connection);
  auto result = m_source->set(row, order_task);
  m_transaction.push(TableModel::UpdateOperation(row, column, previous, value));
  return result;
}

connection OrderTasksToTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

AnyRef OrderTasksToTableModel::extract_field(const OrderTask& order_task,
    Column column) const {
  if(column == Column::NAME) {
    return order_task.m_name;
  } else if(column == Column::REGION) {
    return order_task.m_region;
  } else if(column == Column::DESTINATION) {
    return order_task.m_destination;
  } else if(column == Column::ORDER_TYPE) {
    return order_task.m_order_type;
  } else if(column == Column::SIDE) {
    return order_task.m_side;
  } else if(column == Column::QUANTITY) {
    return order_task.m_quantity;
  } else if(column == Column::TIME_IN_FORCE) {
    return order_task.m_time_in_force;
  } else if(column == Column::KEY) {
    return order_task.m_key;
  }
  return {};
}

void OrderTasksToTableModel::on_operation(
    const ListModel<OrderTask>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<OrderTask>::AddOperation& operation) {
      m_transaction.push(TableModel::AddOperation(operation.m_index,
          to_list(operation.get_value())));
    },
    [&] (const ListModel<OrderTask>::MoveOperation& operation) {
      m_transaction.push(TableModel::MoveOperation(
        operation.m_source, operation.m_destination));
    },
    [&] (const ListModel<OrderTask>::RemoveOperation& operation) {
      m_transaction.push(TableModel::RemoveOperation(operation.m_index,
        to_list(operation.get_value())));
    },
    [&] (const ListModel<OrderTask>::UpdateOperation& operation) {
      for(auto i = 0; i < COLUMN_SIZE; ++i) {
        m_transaction.push(TableModel::UpdateOperation(operation.m_index, i,
          to_any(
            extract_field(operation.get_previous(), static_cast<Column>(i))),
          to_any(
            extract_field(operation.get_value(), static_cast<Column>(i)))));
      }
    });
}
