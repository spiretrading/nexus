#include "Spire/Blotter/BlotterController.hpp"
#include <algorithm>
#include <tuple>
#include <boost/uuid/uuid_generators.hpp>
#include <QEvent>
#include <QMessageBox>
#include "Spire/Blotter/PortfolioSummaryModel.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  bool is_cancelable(const Order& order) {
    return order.get_publisher().with([] (const auto& reports) {
      if(!reports || reports->empty()) {
        return true;
      }
      auto status = reports->back().m_status;
      return !is_terminal(status) && status != OrderStatus::PENDING_CANCEL;
    });
  }

  Side get_side(CancelKeyBindingsModel::Operation operation) {
    using Operation = CancelKeyBindingsModel::Operation;
    if(operation == Operation::MOST_RECENT_ASK ||
        operation == Operation::OLDEST_ASK ||
        operation == Operation::ALL_ASKS ||
        operation == Operation::CLOSEST_ASK ||
        operation == Operation::FURTHEST_ASK) {
      return Side::ASK;
    } else if(operation == Operation::MOST_RECENT_BID ||
        operation == Operation::OLDEST_BID ||
        operation == Operation::ALL_BIDS ||
        operation == Operation::CLOSEST_BID ||
        operation == Operation::FURTHEST_BID) {
      return Side::BID;
    } else {
      return Side::NONE;
    }
  }
}

class BlotterController::EventFilter : public QObject {
  public:
    explicit EventFilter(BlotterController& controller)
      : m_controller(&controller) {}

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::Close) {
        m_controller->on_close();
      }
      return false;
    }

  private:
    BlotterController* m_controller;
};

BlotterController::BlotterController(std::shared_ptr<BlotterModel> model,
  std::shared_ptr<TasksModel> tasks,
  std::shared_ptr<BlotterCollection> collection,
  std::shared_ptr<CancelKeyBindingsModel> cancel_bindings,
  std::shared_ptr<PositionKeyBindingsModel> position_bindings,
  Ref<FlattenController> flatten_controller, Ref<UserProfile> profile)
  : m_id(boost::uuids::random_generator()()),
    m_model(std::move(model)),
    m_tasks(std::move(tasks)),
    m_collection(std::move(collection)),
    m_cancel_bindings(std::move(cancel_bindings)),
    m_position_bindings(std::move(position_bindings)),
    m_flatten_controller(flatten_controller.get()),
    m_profile(profile.get()),
    m_summary(make_proxy_value_model(
      make_constant_value_model(PortfolioUpdateEntry()))),
    m_risk_parameters(std::make_shared<LocalValueModel<RiskParameters>>()),
    m_is_open(std::make_shared<LocalBooleanModel>(false)),
    m_read_only_is_open(m_is_open) {
  m_profile->GetClients().get_administration_client().
    get_risk_parameters_publisher(
      m_model->get_account()).monitor(m_events.get_slot<RiskParameters>(
        std::bind_front(&BlotterController::on_risk_parameters, this)));
}

BlotterController::~BlotterController() {
  m_window_connections.disconnect();
  m_events.close();
  if(m_window) {
    m_window->removeEventFilter(m_event_filter.get());
    m_window.reset();
  }
  if(m_event_filter) {
    m_event_filter.release()->deleteLater();
  }
  if(m_is_open->get()) {
    m_is_open->set(false);
  }
}

const std::shared_ptr<BlotterModel>& BlotterController::get_model() const {
  return m_model;
}

const std::shared_ptr<const BooleanModel>& BlotterController::is_open() const {
  return m_read_only_is_open;
}

std::shared_ptr<Task> BlotterController::add(const CanvasNode& node) {
  auto task =
    std::make_shared<Task>(node, m_model->get_account(), Ref(*m_profile));
  auto entry = BlotterTaskEntry();
  entry.m_task = task;
  m_tasks->push(entry);
  task->GetPublisher().monitor(m_events.get_slot<Task::StateEntry>(
    std::bind_front(&BlotterController::on_state, this, std::weak_ptr(task))));
  return task;
}

std::shared_ptr<Task> BlotterController::submit(const CanvasNode& node) {
  auto task = add(node);
  try {
    task->Execute();
  } catch(const std::exception& e) {
    on_state(task, Task::StateEntry(Task::State::FAILED, e.what()));
    throw;
  }
  return task;
}

void BlotterController::cancel(CancelKeyBindingsModel::Operation operation,
    const Ticker& ticker, const std::unordered_map<OrderId, Money>& prices,
    const optional<std::vector<OrderId>>& ids) {
  auto tasks = std::vector<std::shared_ptr<Task>>();
  auto task_prices = std::unordered_map<std::shared_ptr<Task>, Money>();
  for(auto& entry : std::as_const(*m_model->get_tasks())) {
    if(IsTerminal(entry.m_state) ||
        entry.m_state == Task::State::PENDING_CANCEL) {
      continue;
    }
    auto& task = entry.m_task;
    auto is_match = false;
    if(!ids) {
      if(auto node = task->GetNode().FindChild("ticker")) {
        if(auto value = dynamic_cast<const TickerNode*>(&*node)) {
          is_match = value->GetValue() == ticker;
        }
      }
    }
    task->GetContext().GetOrderPublisher().with([&] (const auto& orders) {
      if(!orders) {
        return;
      }
      for(auto& order : *orders) {
        auto& info = order->get_info();
        if(info.m_fields.m_ticker != ticker ||
            (ids && std::ranges::find(*ids, info.m_id) == ids->end())) {
          continue;
        }
        is_match = true;
        auto price = prices.find(info.m_id);
        if(price != prices.end()) {
          task_prices.insert_or_assign(task, price->second);
        }
      }
    });
    if(is_match) {
      tasks.push_back(task);
    }
  }
  std::ranges::sort(tasks, {}, &Task::GetId);
  execute(operation, task_prices, out(tasks));
}

void BlotterController::flatten(const Ticker& ticker) {
  m_flatten_controller->flatten(*this, ticker);
}

BlotterWindow& BlotterController::open() {
  if(!m_window) {
    if(!m_event_filter) {
      m_event_filter = std::make_unique<EventFilter>(*this);
    }
    m_window = std::make_unique<BlotterWindow>(
      m_model, m_summary, m_risk_parameters, m_collection, m_cancel_bindings,
      m_position_bindings);
    m_window->m_id = m_id;
    m_window_connections.add(m_window->connect_run_tasks_signal(
      std::bind_front(&BlotterController::on_run_tasks, this)));
    m_window_connections.add(m_window->connect_cancel_tasks_signal(
      std::bind_front(&BlotterController::on_cancel_tasks, this)));
    m_window_connections.add(m_window->connect_cancel_task_operation_signal(
      std::bind_front(&BlotterController::on_cancel_task_operation, this)));
    m_window_connections.add(m_window->connect_cancel_orders_signal(
      std::bind_front(&BlotterController::on_cancel_orders, this)));
    m_window_connections.add(m_window->connect_cancel_order_operation_signal(
      std::bind_front(&BlotterController::on_cancel_order_operation, this)));
    m_window_connections.add(m_window->connect_flatten_positions_signal(
      std::bind_front(&BlotterController::on_flatten_positions, this)));
    if(m_window_settings) {
      m_window_settings->Apply(Ref(*m_profile), out(*m_window));
    }
    m_window->installEventFilter(m_event_filter.get());
  }
  m_window->show();
  m_window->raise();
  m_window->activateWindow();
  if(!m_is_open->get()) {
    auto is_open = m_is_open;
    is_open->set(true);
  }
  return *m_window;
}

void BlotterController::close() {
  if(m_window) {
    m_window->close();
  }
}

void BlotterController::on_risk_parameters(const RiskParameters& parameters) {
  m_risk_parameters->set(parameters);
  if(parameters.m_currency ==
      m_summary->get().m_currency_inventory.m_position.m_currency) {
    return;
  }
  if(parameters.m_currency == CurrencyId::NONE) {
    m_summary->set_source(make_constant_value_model(PortfolioUpdateEntry()));
  } else {
    m_summary->set_source(std::make_shared<PortfolioSummaryModel>(
      m_model->get_portfolio(), parameters.m_currency,
      m_profile->GetExchangeRates()));
  }
}

void BlotterController::on_run_tasks(
    const std::vector<std::shared_ptr<Task>>& tasks) {
  for(auto& task : tasks) {
    try {
      task->Execute();
    } catch(const std::exception& e) {
      on_state(task, Task::StateEntry(Task::State::FAILED, e.what()));
      QMessageBox::warning(m_window.get(), QObject::tr("Unable to run task"),
        QString::fromUtf8(e.what()));
    }
  }
}

void BlotterController::on_cancel_tasks(
    const std::vector<std::shared_ptr<Task>>& tasks) {
  for(auto& task : tasks) {
    task->Cancel();
  }
}

void BlotterController::on_cancel_task_operation(
    CancelKeyBindingsModel::Operation operation) {
  auto tasks = std::vector<std::shared_ptr<Task>>();
  for(auto& entry : std::as_const(*m_model->get_tasks())) {
    if(!IsTerminal(entry.m_state) &&
        entry.m_state != Task::State::PENDING_CANCEL) {
      tasks.push_back(entry.m_task);
    }
  }
  std::ranges::sort(tasks, {}, &Task::GetId);
  execute(operation, out(tasks));
}

void BlotterController::on_cancel_orders(
    const std::vector<std::shared_ptr<Order>>& orders) {
  for(auto& order : orders) {
    if(!is_cancelable(*order)) {
      continue;
    }
    try {
      m_profile->GetClients().get_order_execution_client().cancel(order);
    } catch(const std::exception& e) {
      QMessageBox::warning(m_window.get(),
        QObject::tr("Unable to cancel order"), QString::fromUtf8(e.what()));
    }
  }
}

void BlotterController::on_cancel_order_operation(
    CancelKeyBindingsModel::Operation operation) {
  using Operation = CancelKeyBindingsModel::Operation;
  auto side = get_side(operation);
  auto orders = std::vector<std::shared_ptr<Order>>();
  for(auto& order : std::as_const(*m_model->get_orders())) {
    if(is_cancelable(*order) &&
        (side == Side::NONE || order->get_info().m_fields.m_side == side)) {
      orders.push_back(order);
    }
  }
  if(orders.empty()) {
    return;
  }
  std::ranges::sort(orders, [] (const auto& left, const auto& right) {
    auto& a = left->get_info();
    auto& b = right->get_info();
    return std::tie(a.m_timestamp, a.m_id) < std::tie(b.m_timestamp, b.m_id);
  });
  auto selected = orders.begin();
  if(operation == Operation::ALL || operation == Operation::ALL_ASKS ||
      operation == Operation::ALL_BIDS) {
    on_cancel_orders(orders);
    return;
  } else if(operation == Operation::MOST_RECENT ||
      operation == Operation::MOST_RECENT_ASK ||
      operation == Operation::MOST_RECENT_BID) {
    selected = std::prev(orders.end());
  } else if(operation == Operation::CLOSEST_ASK ||
      operation == Operation::CLOSEST_BID ||
      operation == Operation::FURTHEST_ASK ||
      operation == Operation::FURTHEST_BID) {
    auto is_minimum = operation == Operation::CLOSEST_ASK ||
      operation == Operation::FURTHEST_BID;
    auto is_closest = operation == Operation::CLOSEST_ASK ||
      operation == Operation::CLOSEST_BID;
    for(auto i = std::next(orders.begin()); i != orders.end(); ++i) {
      auto price = (*i)->get_info().m_fields.m_price;
      auto selected_price = (*selected)->get_info().m_fields.m_price;
      if((is_minimum && price < selected_price) ||
          (!is_minimum && price > selected_price) ||
          (is_closest && price == selected_price)) {
        selected = i;
      }
    }
  }
  on_cancel_orders({*selected});
}

void BlotterController::on_flatten_positions(
    const std::vector<Position>& positions) {
  for(auto& position : positions) {
    try {
      flatten(position.m_ticker);
    } catch(const std::exception& e) {
      QMessageBox::warning(m_window.get(), QObject::tr("Unable to flatten"),
        QString::fromUtf8(e.what()));
    }
  }
}

void BlotterController::on_state(
    const std::weak_ptr<Task>& task, const Task::StateEntry& state) {
  auto current = task.lock();
  if(!current) {
    return;
  }
  auto i = std::ranges::find(
    std::as_const(*m_tasks), current, &BlotterTaskEntry::m_task);
  if(i != m_tasks->cend()) {
    auto entry = *i;
    entry.m_state = state.m_state;
    m_tasks->set(static_cast<int>(i - m_tasks->cbegin()), entry);
  }
}

void BlotterController::on_close() {
  m_window_connections.disconnect();
  m_window_settings.emplace(*m_window);
  auto window = m_window.release();
  window->removeEventFilter(m_event_filter.get());
  window->deleteLater();
  auto is_open = m_is_open;
  is_open->set(false);
}
