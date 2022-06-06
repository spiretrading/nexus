#include "Spire/Blotter/BlotterOrderLogView.hpp"
#include <unordered_map>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/QtTaskQueue.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct OrderTableModel : TableModel {
    enum Column {
      TIME,
      ID,
      STATUS,
      SECURITY,
      CURRENCY,
      TYPE,
      SIDE,
      DESTINATION,
      QUANTITY,
      PRICE,
      TIME_IN_FORCE
    };
    struct OrderStatusEntry {
      OrderStatus m_status;
      std::vector<int> m_indexes;
    };
    static const auto COLUMN_SIZE = 11;
    QtTaskQueue m_tasks;
    std::shared_ptr<OrderListModel> m_orders;
    std::unordered_map<const Order*, OrderStatusEntry> m_status_entries;
    scoped_connection m_operation_connection;
    TableModelTransactionLog m_transaction;

    OrderTableModel(std::shared_ptr<OrderListModel> orders)
      : m_orders(std::move(orders)),
        m_operation_connection(m_orders->connect_operation_signal(
          std::bind_front(&OrderTableModel::on_operation, this))) {}

    int get_row_size() const override {
      return m_orders->get_size();
    }

    int get_column_size() const override {
      return COLUMN_SIZE;
    }

    AnyRef at(int row, int column) const override {
      if(column < 0 || column >= get_column_size()) {
        throw std::out_of_range("Column is out of range.");
      }
      return extract_field(*m_orders->get(row), column);
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_transaction.connect_operation_signal(slot);
    }

    void add(const Order& order, int index) {
      auto entry = m_status_entries.find(&order);
      if(entry != m_status_entries.end()) {
        entry->second.m_indexes.push_back(index);
        return;
      }
      m_status_entries[&order].m_indexes.push_back(index);
      auto execution_reports = optional<std::vector<ExecutionReport>>();
      order.GetPublisher().Monitor(m_tasks.get_slot<ExecutionReport>(
        std::bind_front(&OrderTableModel::on_execution_report, this,
          std::cref(order))), Store(execution_reports));
      if(execution_reports) {
        for(auto& report : *execution_reports) {
          on_execution_report(order, report);
        }
      }
    }

    void reindex(OrderStatusEntry& entry, const Order& order) {
      entry.m_indexes.clear();
      for(auto i = 0; i != m_orders->get_size(); ++i) {
        if(m_orders->get(i) == &order) {
          entry.m_indexes.push_back(i);
        }
      }
    }

    void on_execution_report(
        const Order& order, const ExecutionReport& report) {
      auto& entry = m_status_entries[&order];
      if(report.m_status == OrderStatus::PARTIALLY_FILLED &&
          entry.m_status == OrderStatus::PENDING_CANCEL) {
        return;
      }
      auto previous = entry.m_status;
      entry.m_status = report.m_status;
      for(auto& index : entry.m_indexes) {
        if(index >= m_orders->get_size() || m_orders->get(index) != &order) {
          reindex(entry, order);
          break;
        }
      }
      m_transaction.transact([&] {
        for(auto& index : entry.m_indexes) {
          m_transaction.push(UpdateOperation(
            index, static_cast<int>(STATUS), previous, entry.m_status));
        }
      });
    }

    void on_operation(const OrderListModel::Operation& operation) {
      m_transaction.transact([&] {
        visit(operation,
          [&] (const OrderListModel::AddOperation& operation) {
            auto row = std::make_shared<ArrayListModel<std::any>>();
            for(auto i = 0; i != get_column_size(); ++i) {
              row->push(to_any(at(operation.m_index, i)));
            }
            m_transaction.push(AddOperation(operation.m_index, row));
            add(*operation.get_value(), operation.m_index);
          },
          [&] (const OrderListModel::RemoveOperation& operation) {
            auto row = std::make_shared<ArrayListModel<std::any>>();
            for(auto i = 0; i != COLUMN_SIZE; ++i) {
              row->push(to_any(extract_field(*operation.get_value(), i)));
            }
            m_transaction.push(RemoveOperation(operation.m_index, row));
          },
          [&] (const OrderListModel::MoveOperation& operation) {
            m_transaction.push(
              MoveOperation(operation.m_source, operation.m_destination));
          },
          [&] (const OrderListModel::UpdateOperation& operation) {
            add(*operation.get_value(), operation.m_index);
            for(auto i = 0; i != COLUMN_SIZE; ++i) {
              m_transaction.push(UpdateOperation(operation.m_index, i,
                to_any(extract_field(*operation.get_previous(), i)),
                to_any(extract_field(*operation.get_value(), i))));
            }
          });
      });
    }

    AnyRef extract_field(const Order& order, int column) const {
      if(column == Column::TIME) {
        return order.GetInfo().m_timestamp;
      } else if(column == Column::ID) {
        return order.GetInfo().m_orderId;
      } else if(column == Column::STATUS) {
        auto status = m_status_entries.find(&order);
        if(status == m_status_entries.end()) {
          static const auto NONE = OrderStatus::NONE;
          return NONE;
        }
        return status->second.m_status;
      } else if(column == Column::SECURITY) {
        return order.GetInfo().m_fields.m_security;
      } else if(column == Column::CURRENCY) {
        return order.GetInfo().m_fields.m_currency;
      } else if(column == Column::TYPE) {
        return order.GetInfo().m_fields.m_type;
      } else if(column == Column::SIDE) {
        return order.GetInfo().m_fields.m_side;
      } else if(column == Column::DESTINATION) {
        return order.GetInfo().m_fields.m_destination;
      } else if(column == Column::QUANTITY) {
        return order.GetInfo().m_fields.m_quantity;
      } else if(column == Column::PRICE) {
        return order.GetInfo().m_fields.m_price;
      } else if(column == Column::TIME_IN_FORCE) {
        return order.GetInfo().m_fields.m_timeInForce;
      }
      return {};
    }
  };
}

BlotterOrderLogView::BlotterOrderLogView(
    std::shared_ptr<OrderListModel> orders, QWidget* parent)
    : QWidget(parent),
      m_orders(std::move(orders)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table_view_builder =
    TableViewBuilder(std::make_shared<OrderTableModel>(m_orders));
  table_view_builder.add_header_item(tr("Time"));
  table_view_builder.add_header_item(tr("ID"));
  table_view_builder.add_header_item(tr("Status"));
  table_view_builder.add_header_item(tr("Security"));
  table_view_builder.add_header_item(tr("Currency"), tr("Curr"));
  table_view_builder.add_header_item(tr("Type"));
  table_view_builder.add_header_item(tr("Side"));
  table_view_builder.add_header_item(tr("Destination"), tr("Dest"));
  table_view_builder.add_header_item(tr("Quantity"), tr("Qty"));
  table_view_builder.add_header_item(tr("Price"), tr("Px"));
  table_view_builder.add_header_item(tr("Time in Force"), tr("TIF"));
  auto table = table_view_builder.make();
  table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto scroll_box = new ScrollBox(table);
  scroll_box->set(
    ScrollBox::DisplayPolicy::ON_OVERFLOW, ScrollBox::DisplayPolicy::NEVER);
  enclose(*this, *scroll_box);
}

const std::shared_ptr<OrderListModel>& BlotterOrderLogView::get_orders() const {
  return m_orders;
}
