#include "Spire/KeyBindings/OrderTaskArgumentsListToTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  static const auto COLUMN_SIZE = 9;

  AnyRef extract(const OrderTaskArguments& arguments, OrderTaskColumns column) {
    if(column == OrderTaskColumns::NAME) {
      return arguments.m_name;
    } else if(column == OrderTaskColumns::REGION) {
      return arguments.m_region;
    } else if(column == OrderTaskColumns::DESTINATION) {
      return arguments.m_destination;
    } else if(column == OrderTaskColumns::ORDER_TYPE) {
      return arguments.m_order_type;
    } else if(column == OrderTaskColumns::SIDE) {
      return arguments.m_side;
    } else if(column == OrderTaskColumns::QUANTITY) {
      return arguments.m_quantity;
    } else if(column == OrderTaskColumns::TIME_IN_FORCE) {
      return arguments.m_time_in_force;
    } else if(column == OrderTaskColumns::TAGS) {
      return arguments.m_additional_tags;
    } else {
      return arguments.m_key;
    }
  }
}

OrderTaskArgumentsListToTableModel::OrderTaskArgumentsListToTableModel(
  std::shared_ptr<OrderTaskArgumentsListModel> order_task_arguments)
  : m_order_task_arguments(std::move(order_task_arguments)),
    m_connection(m_order_task_arguments->connect_operation_signal(
      std::bind_front(
        &OrderTaskArgumentsListToTableModel::on_operation, this))) {}

int OrderTaskArgumentsListToTableModel::get_row_size() const {
  return m_order_task_arguments->get_size();
}

int OrderTaskArgumentsListToTableModel::get_column_size() const {
  return COLUMN_SIZE;
}

AnyRef OrderTaskArgumentsListToTableModel::at(int row, int column) const {
  if(column < 0 || column >= get_column_size()) {
    throw std::out_of_range("The column is out of range.");
  }
 return extract(
    m_order_task_arguments->get(row), static_cast<OrderTaskColumns>(column));
}

QValidator::State OrderTaskArgumentsListToTableModel::set(
    int row, int column, const std::any& value) {
  if(column < 0 || column >= get_column_size()) {
    throw std::out_of_range("The column is out of range.");
  }
  auto column_index = static_cast<OrderTaskColumns>(column);
  auto arguments = m_order_task_arguments->get(row);
  if(column_index == OrderTaskColumns::NAME) {
    arguments.m_name = std::any_cast<const QString&>(value);
  } else if(column_index == OrderTaskColumns::REGION) {
    arguments.m_region = std::any_cast<const Region&>(value);
  } else if(column_index == OrderTaskColumns::DESTINATION) {
    arguments.m_destination = std::any_cast<const Destination&>(value);
  } else if(column_index == OrderTaskColumns::ORDER_TYPE) {
    arguments.m_order_type = std::any_cast<OrderType>(value);
  } else if(column_index == OrderTaskColumns::SIDE) {
    arguments.m_side = std::any_cast<Side>(value);
  } else if(column_index == OrderTaskColumns::QUANTITY) {
    arguments.m_quantity = std::any_cast<QuantitySetting>(value);
  } else if(column_index == OrderTaskColumns::TIME_IN_FORCE) {
    arguments.m_time_in_force = std::any_cast<const TimeInForce&>(value);
  } else if(column_index == OrderTaskColumns::TAGS) {
    arguments.m_additional_tags =
      std::any_cast<const std::vector<AdditionalTag>&>(value);
  } else if(column_index == OrderTaskColumns::KEY) {
    arguments.m_key = std::any_cast<const QKeySequence&>(value);
  }
  return m_order_task_arguments->set(row, std::move(arguments));
}

QValidator::State OrderTaskArgumentsListToTableModel::remove(int row) {
  return m_order_task_arguments->remove(row);
}

connection OrderTaskArgumentsListToTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void OrderTaskArgumentsListToTableModel::on_operation(
    const OrderTaskArgumentsListModel::Operation& operation) {
  visit(operation,
    [&] (const StartTransaction&) {
      m_transaction.start();
    },
    [&] (const EndTransaction&) {
      m_transaction.end();
    },
    [&] (const OrderTaskArgumentsListModel::AddOperation& operation) {
      m_transaction.push(TableModel::AddOperation(operation.m_index));
    },
    [&] (const OrderTaskArgumentsListModel::MoveOperation& operation) {
      m_transaction.push(TableModel::MoveOperation(
        operation.m_source, operation.m_destination));
    },
    [&] (const OrderTaskArgumentsListModel::PreRemoveOperation& operation) {
      m_transaction.push(TableModel::PreRemoveOperation(operation.m_index));
    },
    [&] (const OrderTaskArgumentsListModel::RemoveOperation& operation) {
      m_transaction.push(TableModel::RemoveOperation(operation.m_index));
    },
    [&] (const OrderTaskArgumentsListModel::UpdateOperation& operation) {
      m_transaction.transact([&] {
        for(auto i = 0; i < COLUMN_SIZE; ++i) {
          m_transaction.push(TableModel::UpdateOperation(operation.m_index, i,
            to_any(extract(operation.get_previous(),
              static_cast<OrderTaskColumns>(i))),
            to_any(extract(operation.get_value(),
              static_cast<OrderTaskColumns>(i)))));
        }
      });
  });
}
