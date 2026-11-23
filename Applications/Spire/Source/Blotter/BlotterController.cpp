#include "Spire/Blotter/BlotterController.hpp"
#include <algorithm>
#include <boost/uuid/uuid_generators.hpp>
#include <QEvent>
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

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
  Ref<UserProfile> profile)
  : m_id(boost::uuids::random_generator()()),
    m_model(std::move(model)),
    m_tasks(std::move(tasks)),
    m_collection(std::move(collection)),
    m_cancel_bindings(std::move(cancel_bindings)),
    m_position_bindings(std::move(position_bindings)),
    m_profile(profile.get()),
    m_is_open(std::make_shared<LocalBooleanModel>(false)),
    m_read_only_is_open(m_is_open) {}

BlotterController::~BlotterController() {
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

BlotterWindow& BlotterController::open() {
  if(!m_window) {
    if(!m_event_filter) {
      m_event_filter = std::make_unique<EventFilter>(*this);
    }
    m_window = std::make_unique<BlotterWindow>(
      m_model, m_collection, m_cancel_bindings, m_position_bindings);
    m_window->m_id = m_id;
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
  m_window_settings.emplace(*m_window);
  auto window = m_window.release();
  window->removeEventFilter(m_event_filter.get());
  window->deleteLater();
  auto is_open = m_is_open;
  is_open->set(false);
}
