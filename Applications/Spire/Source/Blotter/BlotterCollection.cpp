#include "Spire/Blotter/BlotterCollection.hpp"
#include <algorithm>
#include <unordered_set>
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/ControllerPortfolioModel.hpp"
#include "Spire/Blotter/MonitoredTasksModel.hpp"
#include "Spire/Spire/UnionListModel.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  struct TaskHash {
    auto operator ()(const BlotterTaskEntry& entry) const noexcept {
      return std::hash<int>()(entry.m_task->GetId());
    }
  };

  struct TaskEqual {
    auto operator ()(const BlotterTaskEntry& left,
        const BlotterTaskEntry& right) const noexcept {
      return left.m_task->GetId() == right.m_task->GetId();
    }
  };

  struct OrderHash {
    auto operator ()(const std::shared_ptr<Order>& order) const noexcept {
      return std::hash<OrderId>()(order->get_info().m_id);
    }
  };

  struct OrderEqual {
    auto operator ()(const std::shared_ptr<Order>& left,
        const std::shared_ptr<Order>& right) const noexcept {
      return left->get_info().m_id == right->get_info().m_id;
    }
  };

  void update_sources(auto& model, auto& sources, const auto& desired) {
    auto next = std::unordered_set(desired.begin(), desired.end());
    model->transact([&] {
      for(auto& source : desired) {
        if(sources.insert(source).second) {
          model->add_source(source);
        }
      }
      for(auto i = sources.begin(); i != sources.end();) {
        if(!next.contains(*i)) {
          model->remove_source(*i);
          i = sources.erase(i);
        } else {
          ++i;
        }
      }
    });
  }
}

struct BlotterCollection::Entry {
  Type m_source;
  Type m_model;
  std::shared_ptr<UnionListModel<BlotterTaskEntry, TaskHash, TaskEqual>>
    m_tasks;
  std::shared_ptr<UnionListModel<std::shared_ptr<Order>, OrderHash, OrderEqual>>
    m_orders;
  std::shared_ptr<ArrayListModel<Type>> m_links;
  std::unordered_set<std::shared_ptr<TasksModel>> m_task_sources;
  std::unordered_set<std::shared_ptr<OrdersModel>> m_order_sources;

  Entry(Type source, MarketDataClient client)
      : m_source(std::move(source)),
        m_tasks(std::make_shared<
          UnionListModel<BlotterTaskEntry, TaskHash, TaskEqual>>()),
        m_orders(std::make_shared<UnionListModel<
          std::shared_ptr<Order>, OrderHash, OrderEqual>>()),
        m_links(std::make_shared<ArrayListModel<Type>>()) {
    update_sources(
      m_tasks, m_task_sources, std::vector{m_source->get_tasks()});
    update_sources(
      m_orders, m_order_sources, std::vector{m_source->get_orders()});
    auto tasks = std::make_shared<MonitoredTasksModel>(
      m_tasks, m_source->get_task_monitors());
    auto pins = std::unordered_set<int>();
    for(auto& entry : std::as_const(*m_source->get_tasks())) {
      if(entry.m_is_pinned) {
        pins.insert(entry.m_task->GetId());
      }
    }
    for(auto i = 0; i != tasks->get_size(); ++i) {
      auto entry = tasks->get(i);
      if(pins.contains(entry.m_task->GetId())) {
        entry.m_is_pinned = true;
        tasks->set(i, entry);
      }
    }
    auto portfolio = [&] () -> std::shared_ptr<PortfolioModel> {
      if(m_source->is_consolidated()) {
        return m_source->get_portfolio();
      }
      return std::make_shared<ControllerPortfolioModel>(m_orders, client);
    }();
    m_model = std::make_shared<CompositeBlotterModel>(m_source->get_account(),
      m_source->is_consolidated(), m_source->get_name(), m_source->is_active(),
      m_source->is_pinned(), tasks, m_source->get_task_monitors(), m_orders,
      std::move(portfolio));
  }
};

BlotterCollection::BlotterCollection(MarketDataClient client)
  : m_client(std::move(client)) {}

BlotterCollection::~BlotterCollection() = default;

BlotterCollection::Type BlotterCollection::add(Type source) {
  for(auto& model : std::as_const(m_blotters)) {
    if(model == source || find(*model)->m_source == source) {
      return model;
    }
    if(source->is_consolidated() && model->is_consolidated() &&
        source->get_account() == model->get_account()) {
      throw std::invalid_argument(
        "The account already has a consolidated blotter.");
    }
  }
  auto entry = std::make_unique<Entry>(std::move(source), m_client);
  auto model = entry->m_model;
  auto pointer = entry.get();
  m_entries.emplace(model.get(), std::move(entry));
  for(auto& current : std::as_const(m_blotters)) {
    if(current->get_account() != model->get_account()) {
      continue;
    }
    if(model->is_consolidated()) {
      pointer->m_links->push(current);
    } else if(current->is_consolidated()) {
      find(*current)->m_links->push(model);
    }
  }
  m_blotters.push(model);
  refresh();
  return model;
}

std::shared_ptr<const ListModel<BlotterCollection::Type>>
    BlotterCollection::get_links(const BlotterModel& blotter) const {
  auto entry = find(blotter);
  if(!entry) {
    throw std::invalid_argument("The blotter is not in the collection.");
  }
  return entry->m_links;
}

QValidator::State BlotterCollection::link(
    const BlotterModel& receiver, const BlotterModel& source) {
  if(!is_valid(receiver, source)) {
    return QValidator::Invalid;
  }
  auto& links = *find(receiver)->m_links;
  auto model = find(source)->m_model;
  if(std::ranges::find(std::as_const(links), model) == links.cend()) {
    links.push(model);
    refresh();
  }
  return QValidator::Acceptable;
}

QValidator::State BlotterCollection::unlink(
    const BlotterModel& receiver, const BlotterModel& source) {
  if(!is_valid(receiver, source)) {
    return QValidator::Invalid;
  }
  auto& links = *find(receiver)->m_links;
  auto model = find(source)->m_model;
  for(auto i = 0; i != links.get_size(); ++i) {
    if(links.get(i) == model) {
      links.remove(i);
      refresh();
      break;
    }
  }
  return QValidator::Acceptable;
}

int BlotterCollection::get_size() const {
  return m_blotters.get_size();
}

const BlotterCollection::Type& BlotterCollection::get(int index) const {
  return m_blotters.get(index);
}

QValidator::State BlotterCollection::remove(int index) {
  auto model = m_blotters.get(index);
  auto entry = find(*model);
  clear(*entry->m_links);
  for(auto& current : std::as_const(m_blotters)) {
    auto& links = *find(*current)->m_links;
    for(auto i = links.get_size() - 1; i >= 0; --i) {
      if(links.get(i) == model) {
        links.remove(i);
      }
    }
  }
  refresh();
  m_blotters.remove(index);
  m_entries.erase(model.get());
  return QValidator::Acceptable;
}

connection BlotterCollection::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_blotters.connect_operation_signal(slot);
}

void BlotterCollection::start_transaction() {
  m_blotters.start_transaction();
}

void BlotterCollection::end_transaction() {
  m_blotters.end_transaction();
}

BlotterCollection::Entry* BlotterCollection::find(
    const BlotterModel& blotter) const {
  auto i = m_entries.find(&blotter);
  if(i == m_entries.end()) {
    return nullptr;
  }
  return i->second.get();
}

bool BlotterCollection::is_valid(
    const BlotterModel& receiver, const BlotterModel& source) const {
  return &receiver != &source && !receiver.is_consolidated() &&
    receiver.get_account() == source.get_account() &&
    find(receiver) && find(source);
}

void BlotterCollection::refresh() {
  for(auto& model : std::as_const(m_blotters)) {
    auto& entry = *find(*model);
    auto visited = std::unordered_set<Entry*>();
    auto pending = std::vector{&entry};
    auto tasks = std::vector<std::shared_ptr<TasksModel>>();
    auto orders = std::vector<std::shared_ptr<OrdersModel>>();
    for(auto i = 0; i != static_cast<int>(pending.size()); ++i) {
      auto current = pending[i];
      if(!visited.insert(current).second) {
        continue;
      }
      tasks.push_back(current->m_source->get_tasks());
      orders.push_back(current->m_source->get_orders());
      for(auto& link : std::as_const(*current->m_links)) {
        pending.push_back(find(*link));
      }
    }
    update_sources(entry.m_tasks, entry.m_task_sources, tasks);
    update_sources(entry.m_orders, entry.m_order_sources, orders);
  }
}
