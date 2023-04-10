#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto to_list(const TimeAndSale& time_and_sale) {
    auto list_model = std::make_shared<ArrayListModel<std::any>>();
    list_model->push(time_and_sale.m_timestamp);
    list_model->push(time_and_sale.m_price);
    list_model->push(time_and_sale.m_size);
    list_model->push(time_and_sale.m_marketCenter);
    list_model->push(time_and_sale.m_condition);
    return list_model;
  }
}

TimeAndSalesTableModel::TimeAndSalesTableModel(
  std::shared_ptr<ListModel<TimeAndSale>> source)
  : m_source(std::move(source)),
    m_source_connection(m_source->connect_operation_signal(
      std::bind_front(&TimeAndSalesTableModel::on_operation, this))) {}

int TimeAndSalesTableModel::get_row_size() const {
  return m_source->get_size();
}

int TimeAndSalesTableModel::get_column_size() const {
  return COLUMN_SIZE;
}

AnyRef TimeAndSalesTableModel::at(int row, int column) const {
  if(column < 0 || column >= get_column_size()) {
    throw std::out_of_range("The column is out of range.");
  }
  return extract_field(m_source->get(row), static_cast<Column>(column));
}

connection TimeAndSalesTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

AnyRef TimeAndSalesTableModel::extract_field(const TimeAndSale& time_and_sale,
    Column column) const {
  if(column == Column::TIME) {
    return time_and_sale.m_timestamp;
  } else if(column == Column::PRICE) {
    return time_and_sale.m_price;
  } else if(column == Column::SIZE) {
    return time_and_sale.m_size;
  } else if(column == Column::MARKET) {
    return time_and_sale.m_marketCenter;
  } else if(column == Column::CONDITION) {
    return time_and_sale.m_condition;
  }
  return {};
}

void TimeAndSalesTableModel::on_operation(
    const ListModel<TimeAndSale>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<TimeAndSale>::AddOperation& operation) {
      m_transaction.push(TableModel::AddOperation(operation.m_index,
        to_list(operation.get_value())));
    },
    [&] (const ListModel<TimeAndSale>::MoveOperation& operation) {
      m_transaction.push(TableModel::MoveOperation(
        operation.m_source, operation.m_destination));
    },
    [&] (const ListModel<TimeAndSale>::RemoveOperation& operation) {
      m_transaction.push(TableModel::RemoveOperation(operation.m_index,
        to_list(operation.get_value())));
    },
    [&] (const ListModel<TimeAndSale>::UpdateOperation& operation) {
      for(auto i = 0; i < COLUMN_SIZE; ++i) {
        m_transaction.push(TableModel::UpdateOperation(operation.m_index, i,
          to_any(
            extract_field(operation.get_previous(), static_cast<Column>(i))),
          to_any(
            extract_field(operation.get_value(), static_cast<Column>(i)))));
      }
    });
}
