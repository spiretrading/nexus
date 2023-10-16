#include "Spire/Blotter/ExecutionReportsToTableModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

namespace {
  AnyRef extract_field(const ExecutionReportEntry& report, int column) {
    if(column == ExecutionReportsToTableModel::Column::TIME) {
      return report.m_executionReport.m_timestamp;
    } else if(column == ExecutionReportsToTableModel::Column::ID) {
      return report.m_order->GetInfo().m_orderId;
    } else if(column == ExecutionReportsToTableModel::Column::SIDE) {
      return report.m_order->GetInfo().m_fields.m_side;
    } else if(column == ExecutionReportsToTableModel::Column::SECURITY) {
      return report.m_order->GetInfo().m_fields.m_security;
    } else if(column == ExecutionReportsToTableModel::Column::ORDER_STATUS) {
      return report.m_executionReport.m_status;
    } else if(column == ExecutionReportsToTableModel::Column::QUANTITY) {
      return report.m_order->GetInfo().m_fields.m_quantity;
    } else if(column == ExecutionReportsToTableModel::Column::LAST_QUANTITY) {
      return report.m_executionReport.m_lastQuantity;
    } else if(column == ExecutionReportsToTableModel::Column::PRICE) {
      return report.m_order->GetInfo().m_fields.m_price;
    } else if(column == ExecutionReportsToTableModel::Column::LAST_PRICE) {
      return report.m_executionReport.m_lastPrice;
    } else if(column == ExecutionReportsToTableModel::Column::MARKET) {
      return report.m_executionReport.m_lastMarket;
    } else if(column == ExecutionReportsToTableModel::Column::LIQUIDITY_FLAG) {
      return report.m_executionReport.m_liquidityFlag;
    } else if(column == ExecutionReportsToTableModel::Column::EXECUTION_FEE) {
      return report.m_executionReport.m_executionFee;
    } else if(column == ExecutionReportsToTableModel::Column::PROCESSING_FEE) {
      return report.m_executionReport.m_processingFee;
    } else if(
        column == ExecutionReportsToTableModel::Column::MISCELLANEOUS_FEE) {
      return report.m_executionReport.m_commission;
    } else if(column == ExecutionReportsToTableModel::Column::MESSAGE) {
      return report.m_executionReport.m_text;
    }
    return {};
  }
}

ExecutionReportsToTableModel::ExecutionReportsToTableModel(
  std::shared_ptr<ExecutionReportListModel> reports)
  : m_reports(std::move(reports)),
    m_connection(m_reports->connect_operation_signal(
      std::bind_front(&ExecutionReportsToTableModel::on_operation, this))) {}

int ExecutionReportsToTableModel::get_row_size() const {
  return m_reports->get_size();
}

int ExecutionReportsToTableModel::get_column_size() const {
  return COLUMN_SIZE;
}

AnyRef ExecutionReportsToTableModel::at(int row, int column) const {
  if(column < 0 || column >= get_column_size()) {
    throw std::out_of_range("Column is out of range.");
  }
  return extract_field(m_reports->get(row), column);
}

connection ExecutionReportsToTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void ExecutionReportsToTableModel::on_operation(
    const ExecutionReportListModel::Operation& operation) {
  m_transaction.transact([&] {
    visit(operation,
      [&] (const ExecutionReportListModel::AddOperation& operation) {
        auto row = std::make_shared<ArrayListModel<std::any>>();
        for(auto i = 0; i != get_column_size(); ++i) {
          row->push(to_any(at(operation.m_index, i)));
        }
        m_transaction.push(AddOperation(operation.m_index, row));
      },
      [&] (const ExecutionReportListModel::RemoveOperation& operation) {
        auto row = std::make_shared<ArrayListModel<std::any>>();
        for(auto i = 0; i != COLUMN_SIZE; ++i) {
          row->push(to_any(
            extract_field(operation.get_value(), static_cast<Column>(i))));
        }
        m_transaction.push(RemoveOperation(operation.m_index, row));
      },
      [&] (const ExecutionReportListModel::MoveOperation& operation) {
        m_transaction.push(
          MoveOperation(operation.m_source, operation.m_destination));
      },
      [&] (const ExecutionReportListModel::UpdateOperation& operation) {
        for(auto i = 0; i != COLUMN_SIZE; ++i) {
          m_transaction.push(UpdateOperation(operation.m_index, i, to_any(
            extract_field(operation.get_previous(), static_cast<Column>(i))),
            to_any(
              extract_field(operation.get_value(), static_cast<Column>(i)))));
        }
      });
  });
}
