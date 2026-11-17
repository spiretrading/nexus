#include "Spire/Blotter/OrderLogView.hpp"
#include <algorithm>
#include <array>
#include <Beam/Queues/ScopedBaseQueue.hpp>
#include "Spire/Async/QtTaskQueue.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  enum class OrderColumn {
    TIME,
    ID,
    STATUS,
    TICKER,
    CURRENCY,
    TYPE,
    SIDE,
    DESTINATION,
    QUANTITY,
    PRICE,
    TIME_IN_FORCE
  };

  AnyRef extract(const std::shared_ptr<Order>& order, int index) {
    auto column = static_cast<OrderColumn>(index);
    auto& info = order->get_info();
    auto& fields = info.m_fields;
    if(column == OrderColumn::TIME) {
      return info.m_timestamp;
    } else if(column == OrderColumn::ID) {
      return info.m_id;
    } else if(column == OrderColumn::TICKER) {
      return fields.m_ticker;
    } else if(column == OrderColumn::CURRENCY) {
      return fields.m_currency;
    } else if(column == OrderColumn::TYPE) {
      return fields.m_type;
    } else if(column == OrderColumn::SIDE) {
      return fields.m_side;
    } else if(column == OrderColumn::DESTINATION) {
      return fields.m_destination;
    } else if(column == OrderColumn::QUANTITY) {
      return fields.m_quantity;
    } else if(column == OrderColumn::PRICE) {
      return make_optional(fields.m_type == OrderType::LIMIT, fields.m_price);
    } else if(column == OrderColumn::TIME_IN_FORCE) {
      return fields.m_time_in_force;
    }
    return QString();
  }

  class OrderLogTableModel : public TableModel {
    public:
      OrderLogTableModel(std::shared_ptr<OrdersModel> orders)
          : m_orders(std::move(orders)) {
        for(auto i = 0; i < m_orders->get_size(); ++i) {
          m_entries.push_back(make_entry(i));
        }
        m_connection = m_orders->connect_operation_signal(
          std::bind_front(&OrderLogTableModel::on_operation, this));
      }

      int get_row_size() const override {
        return static_cast<int>(m_entries.size());
      }

      int get_column_size() const override {
        return static_cast<int>(OrderColumn::TIME_IN_FORCE) + 1;
      }

      AnyRef at(int row, int column) const override {
        if(row < 0 || row >= get_row_size() || column < 0 ||
            column >= get_column_size()) {
          throw std::out_of_range("The row or column is out of range.");
        }
        return extract_entry(*m_entries[row], column);
      }

      QValidator::State remove(int row) override {
        return m_orders->remove(row);
      }

      connection connect_operation_signal(
          const OperationSignal::slot_type& slot) const override {
        return m_transaction.connect_operation_signal(slot);
      }

    private:
      struct Entry {
        std::shared_ptr<Order> m_order;
        int m_index = 0;
        OrderStatus m_status = OrderStatus::PENDING_NEW;
        ScopedBaseQueue<> m_subscription = nullptr;
      };
      std::shared_ptr<OrdersModel> m_orders;
      std::vector<std::shared_ptr<Entry>> m_entries;
      TableModelTransactionLog m_transaction;
      QtTaskQueue m_events;
      scoped_connection m_connection;

      static AnyRef extract_entry(const Entry& entry, int column) {
        if(column == static_cast<int>(OrderColumn::STATUS)) {
          return entry.m_status;
        }
        return extract(entry.m_order, column);
      }

      std::shared_ptr<Entry> make_entry(int index) {
        auto entry = std::make_shared<Entry>();
        entry->m_order = m_orders->get(index);
        entry->m_index = index;
        auto subscription =
          std::make_shared<ScopedQueueWriter<ExecutionReport>>(
            m_events.get_slot<ExecutionReport>(
              [=, this, entry = std::weak_ptr(entry)] (const auto& report) {
                if(auto current = entry.lock()) {
                  on_report(*current, report);
                }
              }));
        entry->m_subscription = ScopedBaseQueue(subscription);
        auto snapshot = optional<std::vector<ExecutionReport>>();
        entry->m_order->get_publisher().monitor(
          std::move(subscription), out(snapshot));
        if(snapshot && !snapshot->empty()) {
          entry->m_status = snapshot->back().m_status;
        }
        return entry;
      }

      void reindex(int first) {
        for(auto i = first; i < get_row_size(); ++i) {
          m_entries[i]->m_index = i;
        }
      }

      void on_report(Entry& entry, const ExecutionReport& report) {
        if(entry.m_status == report.m_status) {
          return;
        }
        auto previous = std::exchange(entry.m_status, report.m_status);
        m_transaction.push(UpdateOperation(entry.m_index,
          static_cast<int>(OrderColumn::STATUS), previous, entry.m_status));
      }

      void on_operation(const OrdersModel::Operation& operation) {
        visit(operation,
          [&] (const OrdersModel::AddOperation& operation) {
            m_entries.insert(m_entries.begin() + operation.m_index,
              make_entry(operation.m_index));
            reindex(operation.m_index);
            m_transaction.push(AddOperation(operation.m_index));
          },
          [&] (const OrdersModel::MoveOperation& operation) {
            auto first = m_entries.begin() + operation.m_source;
            auto last = m_entries.begin() + operation.m_destination;
            if(first < last) {
              std::ranges::rotate(first, first + 1, last + 1);
            } else {
              std::ranges::rotate(last, first, first + 1);
            }
            reindex(std::min(operation.m_source, operation.m_destination));
            m_transaction.push(MoveOperation(
              operation.m_source, operation.m_destination));
          },
          [&] (const OrdersModel::RemoveOperation& operation) {
            m_entries.erase(m_entries.begin() + operation.m_index);
            reindex(operation.m_index);
            m_transaction.push(RemoveOperation(operation.m_index));
          },
          [&] (const OrdersModel::UpdateOperation& operation) {
            auto previous = std::exchange(m_entries[operation.m_index],
              make_entry(operation.m_index));
            auto& entry = *m_entries[operation.m_index];
            m_transaction.transact([&] {
              for(auto i = 0; i < get_column_size(); ++i) {
                m_transaction.push(UpdateOperation(operation.m_index, i,
                  to_any(extract_entry(*previous, i)),
                  to_any(extract_entry(entry, i))));
              }
            });
          },
          [&] (const auto& operation) {
            m_transaction.push(to_table_operation(operation));
          });
      }
  };

}

OrderLogView::OrderLogView(std::shared_ptr<OrdersModel> orders, QWidget* parent)
    : QWidget(parent),
      m_orders(std::move(orders)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table = std::make_shared<OrderLogTableModel>(m_orders);
  auto table_view = TableViewBuilder(table).
    add_header_item(tr("Time"), TableHeaderItem::Order::DESCENDING).
    add_header_item(tr("ID")).
    add_header_item(tr("Status")).
    add_header_item(tr("Ticker")).
    add_header_item(tr("Currency"), tr("Curr")).
    add_header_item(tr("Type")).
    add_header_item(tr("Side")).
    add_header_item(tr("Destination"), tr("Dest")).
    add_header_item(tr("Quantity"), tr("Qty")).
    add_header_item(tr("Price"), tr("Px")).
    add_header_item(tr("Time In Force"), tr("TIF")).
    set_item_builder([] (const auto& table, auto row, auto index) {
      auto column = static_cast<OrderColumn>(index);
      auto item = [&] () -> QWidget* {
        if(column == OrderColumn::SIDE) {
          return make_label(make_to_text_model(
            make_table_value_model<Side>(table, row, index),
            [] (const auto& side) {
              if(side == Side::BID) {
                return tr("Bid");
              } else if(side == Side::ASK) {
                return tr("Ask");
              }
              return QString();
            }));
        }
        return TableView::default_item_builder(table, row, index);
      }();
      if(column != OrderColumn::QUANTITY && column != OrderColumn::PRICE) {
        update_style(*item, [] (auto& style) {
          style.get(Any()).set(TextAlign(Qt::AlignCenter));
        });
      }
      return item;
    }).
    make();
  const auto DEFAULT_WIDTHS =
    std::array{110, 100, 100, 105, 90, 75, 50, 90, 70, 70, 90};
  for(auto i = 0; i < table->get_column_size(); ++i) {
    table_view->get_header().get_widths()->set(
      i, scale_width(DEFAULT_WIDTHS[i]));
    auto column = static_cast<OrderColumn>(i);
    if(column != OrderColumn::QUANTITY && column != OrderColumn::PRICE) {
      update_style(*table_view->get_header().get_item(i), [] (auto& style) {
        style.get(Any() > TableHeaderItem::Label()).
          set(TextAlign(Qt::AlignCenter));
      });
    }
  }
  table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *table_view);
  setFocusProxy(table_view);
}

const std::shared_ptr<OrdersModel>& OrderLogView::get_orders() const {
  return m_orders;
}
