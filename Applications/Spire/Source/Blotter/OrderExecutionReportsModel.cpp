#include "Spire/Blotter/OrderExecutionReportsModel.hpp"
#include <algorithm>
#include <unordered_set>
#include <Beam/Queues/ScopedBaseQueue.hpp>

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

struct OrderExecutionReportsModel::OrderEntry {
  std::shared_ptr<Order> m_order;
  std::unordered_set<int> m_sequences;
  ScopedBaseQueue<> m_subscription;

  OrderEntry(std::shared_ptr<Order> order)
    : m_order(std::move(order)),
      m_subscription(nullptr) {}
};

OrderExecutionReportsModel::OrderExecutionReportsModel(
    std::shared_ptr<OrdersModel> orders)
    : m_orders(std::move(orders)) {
  for(auto i = 0; i != m_orders->get_size(); ++i) {
    add(i, m_orders->get(i));
  }
  m_orders_connection = m_orders->connect_operation_signal(
    std::bind_front(&OrderExecutionReportsModel::on_operation, this));
}

OrderExecutionReportsModel::~OrderExecutionReportsModel() = default;

int OrderExecutionReportsModel::get_size() const {
  return m_reports.get_size();
}

const OrderExecutionReportsModel::Type&
    OrderExecutionReportsModel::get(int index) const {
  return m_reports.get(index);
}

connection OrderExecutionReportsModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_reports.connect_operation_signal(slot);
}

void OrderExecutionReportsModel::start_transaction() {
  m_reports.start_transaction();
}

void OrderExecutionReportsModel::end_transaction() {
  m_reports.end_transaction();
}

void OrderExecutionReportsModel::add(int index, std::shared_ptr<Order> order) {
  auto entry = std::make_shared<OrderEntry>(std::move(order));
  m_entries.insert(m_entries.begin() + index, entry);
  auto subscription = std::make_shared<ScopedQueueWriter<ExecutionReport>>(
    m_events.get_slot<ExecutionReport>(
      [=, this, weak_entry = std::weak_ptr(entry)] (const auto& report) {
        if(auto entry = weak_entry.lock()) {
          on_report(*entry, report);
        }
      }));
  entry->m_subscription = ScopedBaseQueue(subscription);
  auto snapshot = optional<std::vector<ExecutionReport>>();
  entry->m_order->get_publisher().monitor(
    std::move(subscription), out(snapshot));
  if(snapshot) {
    for(auto& report : *snapshot) {
      on_report(*entry, report);
    }
  }
}

void OrderExecutionReportsModel::remove(const OrderEntry& entry) {
  auto id = entry.m_order->get_info().m_id;
  auto counts = m_counts.find(id);
  if(counts == m_counts.end()) {
    return;
  }
  for(auto i = m_reports.get_size() - 1; i >= 0; --i) {
    auto& report = m_reports.get(i).m_report;
    if(report.m_id == id && entry.m_sequences.contains(report.m_sequence)) {
      auto count = counts->second.find(report.m_sequence);
      --count->second;
      if(count->second == 0) {
        counts->second.erase(count);
        m_reports.remove(i);
      }
    }
  }
  if(counts->second.empty()) {
    m_counts.erase(counts);
  }
}

void OrderExecutionReportsModel::on_report(
    OrderEntry& entry, const ExecutionReport& report) {
  if(!entry.m_sequences.insert(report.m_sequence).second) {
    return;
  }
  auto& count = m_counts[report.m_id][report.m_sequence];
  ++count;
  if(count == 1) {
    m_reports.push(ExecutionReportEntry(entry.m_order, report));
  }
}

void OrderExecutionReportsModel::on_operation(
    const OrdersModel::Operation& operation) {
  visit(operation,
    [&] (const OrdersModel::StartTransaction&) {
      start_transaction();
    },
    [&] (const OrdersModel::EndTransaction&) {
      end_transaction();
    },
    [&] (const OrdersModel::AddOperation& operation) {
      transact([&] {
        add(operation.m_index, m_orders->get(operation.m_index));
      });
    },
    [&] (const OrdersModel::MoveOperation& operation) {
      auto first = m_entries.begin() + operation.m_source;
      auto last = m_entries.begin() + operation.m_destination;
      if(first < last) {
        std::ranges::rotate(first, first + 1, last + 1);
      } else {
        std::ranges::rotate(last, first, first + 1);
      }
    },
    [&] (const OrdersModel::RemoveOperation& operation) {
      auto entry = std::move(m_entries[operation.m_index]);
      m_entries.erase(m_entries.begin() + operation.m_index);
      transact([&] { remove(*entry); });
    },
    [&] (const OrdersModel::UpdateOperation& operation) {
      if(operation.get_previous() == operation.get_value()) {
        return;
      }
      transact([&] {
        auto entry = std::move(m_entries[operation.m_index]);
        m_entries.erase(m_entries.begin() + operation.m_index);
        add(operation.m_index, operation.get_value());
        remove(*entry);
      });
    },
    [] (const auto&) {});
}
