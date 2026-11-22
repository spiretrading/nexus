#include "Spire/Blotter/ControllerPortfolioModel.hpp"
#include <algorithm>
#include <unordered_set>
#include <Beam/Queues/Queue.hpp>

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

struct ControllerPortfolioModel::Source {
  std::shared_ptr<OrdersModel> m_orders;
  Controller::Portfolio m_initial;
  MarketDataClient m_client;
  std::shared_ptr<Queue<std::shared_ptr<Order>>> m_queue;
  std::unordered_set<OrderId> m_ids;
  std::vector<std::shared_ptr<Order>> m_pending;
  int m_depth;
  bool m_is_rebuild;

  Source(std::shared_ptr<OrdersModel> orders, Controller::Portfolio initial,
    MarketDataClient client)
    : m_orders(std::move(orders)),
      m_initial(std::move(initial)),
      m_client(std::move(client)),
      m_depth(0),
      m_is_rebuild(false) {}
};

ControllerPortfolioModel::ControllerPortfolioModel(
    std::shared_ptr<Controller> controller)
    : m_controller(std::move(controller)) {
  monitor();
}

ControllerPortfolioModel::ControllerPortfolioModel(
    std::shared_ptr<OrdersModel> orders, MarketDataClient client)
  : ControllerPortfolioModel(
      std::move(orders), Controller::Portfolio(), std::move(client)) {}

ControllerPortfolioModel::ControllerPortfolioModel(
    std::shared_ptr<OrdersModel> orders, Controller::Portfolio initial,
    MarketDataClient client)
    : m_source(std::make_unique<Source>(
        std::move(orders), std::move(initial), std::move(client))) {
  rebuild();
  m_orders_connection = m_source->m_orders->connect_operation_signal(
    std::bind_front(&ControllerPortfolioModel::on_orders, this));
}

ControllerPortfolioModel::~ControllerPortfolioModel() = default;

int ControllerPortfolioModel::get_size() const {
  return m_entries.get_size();
}

const ControllerPortfolioModel::Type&
    ControllerPortfolioModel::get(int index) const {
  return m_entries.get(index);
}

connection ControllerPortfolioModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_entries.connect_operation_signal(slot);
}

void ControllerPortfolioModel::start_transaction() {
  m_entries.start_transaction();
}

void ControllerPortfolioModel::end_transaction() {
  m_entries.end_transaction();
}

void ControllerPortfolioModel::monitor() {
  auto& publisher = m_controller->get_publisher();
  auto controller = std::weak_ptr(m_controller);
  publisher.with([&] {
    auto snapshot = optional<Controller::Portfolio*>();
    publisher.monitor(m_events.get_slot<Type>([=, this] (const auto& update) {
      if(auto current = controller.lock()) {
        if(current == m_controller) {
          on_update(update);
        }
      }
    }), out(snapshot));
    if(snapshot) {
      for_each(**snapshot,
        std::bind_front(&ControllerPortfolioModel::on_update, this));
    }
  });
}

void ControllerPortfolioModel::rebuild() {
  auto queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
  auto ids = std::unordered_set<OrderId>();
  for(auto& order : std::as_const(*m_source->m_orders)) {
    if(ids.insert(order->get_info().m_id).second) {
      queue->push(order);
    }
  }
  auto controller = std::make_shared<Controller>(
    m_source->m_initial, m_source->m_client, queue);
  m_controller = std::move(controller);
  m_source->m_queue = std::move(queue);
  m_source->m_ids = std::move(ids);
  m_source->m_pending.clear();
  m_source->m_is_rebuild = false;
  m_entries.transact([&] {
    while(m_entries.get_size() != 0) {
      m_entries.remove(m_entries.get_size() - 1);
    }
    m_indices.clear();
    monitor();
  });
}

void ControllerPortfolioModel::flush() {
  if(m_source->m_depth != 0) {
    return;
  }
  auto has_history = std::ranges::any_of(m_source->m_pending,
    [] (const auto& order) {
      auto has_history = false;
      order->get_publisher().with([&] (auto reports) {
        if(reports) {
          has_history = std::ranges::any_of(*reports, [] (const auto& report) {
            return report.m_last_quantity != 0;
          });
        }
      });
      return has_history;
    });
  if(m_source->m_is_rebuild || has_history) {
    rebuild();
  } else {
    auto pending = std::exchange(m_source->m_pending, {});
    for(auto& order : pending) {
      if(m_source->m_ids.insert(order->get_info().m_id).second) {
        m_source->m_queue->push(order);
      }
    }
  }
}

void ControllerPortfolioModel::on_orders(
    const OrdersModel::Operation& operation) {
  visit(operation,
    [&] (OrdersModel::StartTransaction) {
      ++m_source->m_depth;
    },
    [&] (OrdersModel::EndTransaction) {
      --m_source->m_depth;
      flush();
    },
    [&] (const OrdersModel::AddOperation& add) {
      m_source->m_pending.push_back(m_source->m_orders->get(add.m_index));
      flush();
    },
    [&] (const OrdersModel::RemoveOperation&) {
      m_source->m_is_rebuild = true;
      flush();
    },
    [&] (const OrdersModel::UpdateOperation& update) {
      if(update.get_previous() != update.get_value()) {
        m_source->m_is_rebuild = true;
        flush();
      }
    });
}

void ControllerPortfolioModel::on_update(const PortfolioUpdateEntry& update) {
  auto [entry, is_inserted] = m_indices.try_emplace(
    update.m_inventory.m_position.m_ticker, m_entries.get_size());
  if(is_inserted) {
    m_entries.push(update);
  } else {
    m_entries.set(entry->second, update);
  }
}
