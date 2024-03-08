#include "Spire/KeyBindings/OrderTaskArgumentsToTableModel.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto to_list(const OrderTaskArguments& arguments) {
    auto list_model = std::make_shared<ArrayListModel<std::any>>();
    list_model->push(arguments.m_name);
    list_model->push(arguments.m_region);
    list_model->push(arguments.m_destination);
    list_model->push(arguments.m_order_type);
    list_model->push(arguments.m_side);
    list_model->push(arguments.m_quantity);
    list_model->push(arguments.m_time_in_force);
    list_model->push(arguments.m_additional_tags);
    list_model->push(arguments.m_key);
    return list_model;
  }
}

OrderTaskArgumentsToTableModel::OrderTaskArgumentsToTableModel(
  std::shared_ptr<OrderTaskArgumentsListModel> source)
  : m_source(std::move(source)),
    m_source_connection(m_source->connect_operation_signal(
      std::bind_front(&OrderTaskArgumentsToTableModel::on_operation, this))) {}

int OrderTaskArgumentsToTableModel::get_row_size() const {
  return m_source->get_size();
}

int OrderTaskArgumentsToTableModel::get_column_size() const {
  return COLUMN_SIZE;
}

AnyRef OrderTaskArgumentsToTableModel::at(int row, int column) const {
  if(column < 0 || column >= get_column_size()) {
    throw std::out_of_range("The column is out of range.");
  }
  return extract_field(m_source->get(row), static_cast<Column>(column));
}

QValidator::State OrderTaskArgumentsToTableModel::set(int row, int column,
    const std::any& value) {
  if(column < 0 || column >= get_column_size()) {
    throw std::out_of_range("The column is out of range.");
  }
  auto column_index = static_cast<Column>(column);
  auto arguments = m_source->get(row);
  auto previous = to_any(extract_field(arguments, column_index));
  if(column_index == Column::NAME) {
    arguments.m_name = std::any_cast<QString>(value);
  } else if(column_index == Column::REGION) {
    arguments.m_region = std::any_cast<Region>(value);
  } else if(column_index == Column::DESTINATION) {
    arguments.m_destination = std::any_cast<Destination>(value);
  } else if(column_index == Column::ORDER_TYPE) {
    arguments.m_order_type = std::any_cast<OrderType>(value);
  } else if(column_index == Column::SIDE) {
    arguments.m_side = std::any_cast<Side>(value);
  } else if(column_index == Column::QUANTITY) {
    arguments.m_quantity = std::any_cast<optional<Quantity>>(value);
  } else if(column_index == Column::TIME_IN_FORCE) {
    arguments.m_time_in_force = std::any_cast<TimeInForce>(value);
  } else if(column_index == Column::TAG) {
    arguments.m_additional_tags = {};
  } else if(column_index == Column::KEY) {
    arguments.m_key = std::any_cast<QKeySequence>(value);
  }
  auto blocker = shared_connection_block(m_source_connection);
  auto result = m_source->set(row, arguments);
  m_transaction.push(TableModel::UpdateOperation(row, column, previous, value));
  return result;
}

connection OrderTaskArgumentsToTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

AnyRef OrderTaskArgumentsToTableModel::extract_field(
    const OrderTaskArguments& arguments, Column column) const {
  if(column == Column::NAME) {
    return arguments.m_name;
  } else if(column == Column::REGION) {
    return arguments.m_region;
  } else if(column == Column::DESTINATION) {
    return arguments.m_destination;
  } else if(column == Column::ORDER_TYPE) {
    return arguments.m_order_type;
  } else if(column == Column::SIDE) {
    return arguments.m_side;
  } else if(column == Column::QUANTITY) {
    return arguments.m_quantity;
  } else if(column == Column::TIME_IN_FORCE) {
    return arguments.m_time_in_force;
  } else if(column == Column::TAG) {
    return arguments.m_additional_tags;
  } else if(column == Column::KEY) {
    return arguments.m_key;
  }
  return {};
}

void OrderTaskArgumentsToTableModel::on_operation(
    const OrderTaskArgumentsListModel::Operation& operation) {
  visit(operation,
    [&] (const OrderTaskArgumentsListModel::AddOperation& operation) {
      m_transaction.push(TableModel::AddOperation(operation.m_index,
        to_list(operation.get_value())));
    },
    [&] (const OrderTaskArgumentsListModel::MoveOperation& operation) {
      m_transaction.push(TableModel::MoveOperation(
        operation.m_source, operation.m_destination));
    },
    [&] (const OrderTaskArgumentsListModel::RemoveOperation& operation) {
      m_transaction.push(TableModel::RemoveOperation(operation.m_index,
        to_list(operation.get_value())));
    },
    [&] (const OrderTaskArgumentsListModel::UpdateOperation& operation) {
      for(auto i = 0; i < COLUMN_SIZE; ++i) {
        m_transaction.push(TableModel::UpdateOperation(operation.m_index, i,
          to_any(
            extract_field(operation.get_previous(), static_cast<Column>(i))),
          to_any(
            extract_field(operation.get_value(), static_cast<Column>(i)))));
      }
    });
}
