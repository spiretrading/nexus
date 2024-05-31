#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  AnyRef extract_field(const TimeAndSale& time_and_sale,
      TimeAndSalesTableModel::Column column) {
    if(column == TimeAndSalesTableModel::Column::TIME) {
      return time_and_sale.m_timestamp;
    } else if(column == TimeAndSalesTableModel::Column::PRICE) {
      return time_and_sale.m_price;
    } else if(column == TimeAndSalesTableModel::Column::SIZE) {
      return time_and_sale.m_size;
    } else if(column == TimeAndSalesTableModel::Column::MARKET) {
      return time_and_sale.m_marketCenter;
    } else {
      return time_and_sale.m_condition;
    }
  }
}

TimeAndSalesTableModel::TimeAndSalesTableModel(
  std::shared_ptr<TimeAndSalesModel> model)
  : m_model(std::move(model)),
    m_connection(m_model->connect_update_signal(
      std::bind_front(&TimeAndSalesTableModel::on_update, this))) {}

const std::shared_ptr<TimeAndSalesModel>&
    TimeAndSalesTableModel::get_model() const {
  return m_model;
}

void TimeAndSalesTableModel::set_model(
    std::shared_ptr<TimeAndSalesModel> model) {
  auto size = get_row_size();
  m_transaction.transact([&] {
    for(auto i = size - 1; i >= 0; --i) {
      m_transaction.push(TableModel::RemoveOperation(i));
    }
  });
  m_entries.clear();
  m_model = std::move(model);
  m_connection = m_model->connect_update_signal(
    std::bind_front(&TimeAndSalesTableModel::on_update, this));
}

void TimeAndSalesTableModel::load_history(int max_count) {
  if(m_entries.empty()) {
    load_snapshot(Queries::Sequence::Present(), max_count);
  } else {
    load_snapshot(m_entries.back().m_time_and_sale.GetSequence(), max_count);
  }
}

BboIndicator TimeAndSalesTableModel::get_bbo_indicator(int row) const {
  if(row < 0 || row >= get_row_size()) {
    throw std::out_of_range("The row is out of range.");
  }
  return m_entries[row].m_indicator;
}

int TimeAndSalesTableModel::get_row_size() const {
  return static_cast<int>(m_entries.size());
}

int TimeAndSalesTableModel::get_column_size() const {
  return COLUMN_SIZE;
}

AnyRef TimeAndSalesTableModel::at(int row, int column) const {
  if(column < 0 || column >= get_column_size()) {
    throw std::out_of_range("The column is out of range.");
  }
  return extract_field(m_entries[row].m_time_and_sale.GetValue(),
    static_cast<Column>(column));
}

connection TimeAndSalesTableModel::connect_begin_loading_signal(
    const BeginLoadingSignal::slot_type& slot) const {
  return m_begin_loading_signal.connect(slot);
}

connection TimeAndSalesTableModel::connect_end_loading_signal(
    const EndLoadingSignal::slot_type& slot) const {
  return m_end_loading_signal.connect(slot);
}

connection TimeAndSalesTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void TimeAndSalesTableModel::load_snapshot(Queries::Sequence last, int count) {
  m_begin_loading_signal();
  m_promise = m_model->query_until(last, count).then(
    [=] (auto&& result) {
      auto& snapshot = result.Get();
      m_transaction.transact([&] {
        for(auto i = snapshot.rbegin(); i != snapshot.rend(); ++i) {
          m_entries.push_back(*i);
          m_transaction.push(TableModel::AddOperation(m_entries.size() - 1));
        }
      });
      m_end_loading_signal();
    });
}

void TimeAndSalesTableModel::on_update(const TimeAndSalesModel::Entry& entry) {
  m_entries.push_front(entry);
  m_transaction.push(TableModel::AddOperation(0));
}
