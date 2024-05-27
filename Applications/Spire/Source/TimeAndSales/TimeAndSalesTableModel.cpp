#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  AnyRef extract_field(const TimeAndSale& time_and_sale, TimeAndSalesTableModel::Column column) {
    if(column == TimeAndSalesTableModel::Column::TIME) {
      return time_and_sale.m_timestamp;
    } else if(column == TimeAndSalesTableModel::Column::PRICE) {
      return time_and_sale.m_price;
    } else if(column == TimeAndSalesTableModel::Column::SIZE) {
      return time_and_sale.m_size;
    } else if(column == TimeAndSalesTableModel::Column::MARKET) {
      return time_and_sale.m_marketCenter;
    } else if(column == TimeAndSalesTableModel::Column::CONDITION) {
      return time_and_sale.m_condition;
    }
    return {};
  }
}

TimeAndSalesTableModel::TimeAndSalesTableModel(
    std::shared_ptr<TimeAndSalesModel> model)
    : m_model(std::move(model)),
      m_source_connection(m_model->connect_update_signal(
        std::bind_front(&TimeAndSalesTableModel::on_update, this))) {
  m_entries.connect_operation_signal(
    std::bind_front(&TimeAndSalesTableModel::on_operation, this));
}

const std::shared_ptr<TimeAndSalesModel>& TimeAndSalesTableModel::get_model() const {
  return m_model;
}

void TimeAndSalesTableModel::set_model(std::shared_ptr<TimeAndSalesModel> model) {
  clear(m_entries);
  m_model = std::move(model);
  m_source_connection = m_model->connect_update_signal(
    std::bind_front(&TimeAndSalesTableModel::on_update, this));
}

void TimeAndSalesTableModel::load_history(int max_count) {
  if(m_entries.get_size() == 0) {
    load_snapshot(Beam::Queries::Sequence::Present(), max_count);
  } else {
    load_snapshot(
      m_entries.get(m_entries.get_size() - 1).m_time_and_sale.GetSequence(),
      max_count);
  }
}
//
//BboIndicator TimeAndSalesTableModel::get_bbo_indicator(int row) const {
//  return m_entries.get(row).m_indicator;
//}

int TimeAndSalesTableModel::get_row_size() const {
  return m_entries.get_size();
}

int TimeAndSalesTableModel::get_column_size() const {
  return COLUMN_SIZE;
}

AnyRef TimeAndSalesTableModel::at(int row, int column) const {
  if(column < 0 || column >= get_column_size()) {
    throw std::out_of_range("The column is out of range.");
  }
  return extract_field(m_entries.get(row).m_time_and_sale.GetValue(),
    static_cast<Column>(column));
}

connection TimeAndSalesTableModel::connect_begin_loading_signal(const BeginLoadingSignal::slot_type& slot) const {
  return m_begin_loading_signal.connect(slot);
}

connection TimeAndSalesTableModel::connect_end_loading_signal(const EndLoadingSignal::slot_type& slot) const {
  return m_end_loading_signal.connect(slot);
}

connection TimeAndSalesTableModel::connect_operation_signal(const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void TimeAndSalesTableModel::load_snapshot(Queries::Sequence last, int count) {
  m_begin_loading_signal();
  m_promise = m_model->query_until(last, count).then(
    [=] (auto&& result) {
      auto& snapshot = result.Get();
      for(auto i = snapshot.rbegin(); i != snapshot.rend(); ++i) {
        m_entries.push(*i);
      }
      m_end_loading_signal();
    });
}

void TimeAndSalesTableModel::on_update(const TimeAndSalesModel::Entry& entry) {
  m_entries.insert(entry, 0);
}

void TimeAndSalesTableModel::on_operation(
  const ListModel<TimeAndSalesModel::Entry>::Operation& operation) {
  visit(operation,
    [&] (const StartTransaction&) {
      m_transaction.start();
    },
    [&] (const EndTransaction&) {
      m_transaction.end();
    },
    [&] (const ListModel<TimeAndSalesModel::Entry>::AddOperation& operation) {
      m_transaction.push(TableModel::AddOperation(operation.m_index));
    },
    [&] (const ListModel<TimeAndSalesModel::Entry>::RemoveOperation& operation) {
      m_transaction.push(TableModel::RemoveOperation(operation.m_index));
    });
}
