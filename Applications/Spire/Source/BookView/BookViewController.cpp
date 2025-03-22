#include "Spire/BookView/BookViewController.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/BookView/ServiceBookViewModel.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include "Spire/Canvas/Operations/FindCanvasNodeValue.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

namespace {
  optional<Security> find_security(const CanvasNode& node) {
    return find_value<SecurityNode>(
      node, SingleOrderTaskNode::SECURITY_PROPERTY);
  }

  optional<Money> find_price(const CanvasNode& node) {
    return find_value<MoneyNode>(node, SingleOrderTaskNode::PRICE_PROPERTY);
  }

  optional<Destination> find_destination(const CanvasNode& node) {
    return find_value<DestinationNode>(
      node, SingleOrderTaskNode::DESTINATION_PROPERTY);
  }
}

struct BookViewController::EventFilter : QObject {
  BookViewController* m_controller;

  EventFilter(BookViewController& controller)
    : m_controller(&controller) {}

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::Close) {
      m_controller->close();
    }
    return QObject::eventFilter(watched, event);
  }
};

BookViewController::BookViewController(Ref<UserProfile> user_profile)
  : m_user_profile(user_profile.Get()),
    m_window(nullptr) {}

BookViewController::BookViewController(
    Ref<UserProfile> user_profile, BookViewWindow& window)
    : m_user_profile(user_profile.Get()),
      m_window(&window) {
  m_event_filter = std::make_unique<EventFilter>(*this);
  m_window->installEventFilter(m_event_filter.get());
  m_submit_task_connection = m_window->connect_submit_task_signal(
    std::bind_front(&BookViewController::on_submit_task, this));
  m_cancel_operation_connection = m_window->connect_cancel_operation_signal(
    std::bind_front(&BookViewController::on_cancel_operation, this));
}

BookViewController::~BookViewController() {
  close();
}

void BookViewController::open() {
  if(m_window) {
    m_window->show();
    return;
  }
  m_window = new BookViewWindow(Ref(*m_user_profile),
    m_user_profile->GetSecurityInfoQueryModel(),
    m_user_profile->GetKeyBindings(), m_user_profile->GetMarketDatabase(),
    m_user_profile->GetBookViewPropertiesWindowFactory(),
    m_user_profile->GetBookViewModelBuilder());
  m_event_filter = std::make_unique<EventFilter>(*this);
  m_window->installEventFilter(m_event_filter.get());
  m_submit_task_connection = m_window->connect_submit_task_signal(
    std::bind_front(&BookViewController::on_submit_task, this));
  m_cancel_operation_connection = m_window->connect_cancel_operation_signal(
    std::bind_front(&BookViewController::on_cancel_operation, this));
  m_window->show();
}

void BookViewController::close() {
  if(!m_window) {
    return;
  }
  m_window->removeEventFilter(m_event_filter.get());
  m_window->close();
  m_window->deleteLater();
  m_window = nullptr;
  m_closed_signal();
}

connection BookViewController::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void BookViewController::on_submit_task(
    const std::shared_ptr<CanvasNode>& task) {
  auto errors = Validate(*task);
  if(!errors.empty()) {
    return;
  }
  auto& active_blotter =
    m_user_profile->GetBlotterSettings().GetActiveBlotter();
  auto& blotter_window =
    BlotterWindow::GetBlotterWindow(Ref(*m_user_profile), Ref(active_blotter));
  if(!blotter_window.isVisible()) {
    blotter_window.setAttribute(Qt::WA_ShowWithoutActivating);
    blotter_window.show();
    if(m_window) {
      m_window->raise();
    }
  }
  auto& entry = active_blotter.GetTasksModel().Add(*task);
  entry.m_task->Execute();
}

void BookViewController::on_cancel_operation(
    CancelKeyBindingsModel::Operation operation, const Security& security,
    const optional<BookViewWindow::CancelCriteria>& criteria) {
  auto& tasks_model =
    m_user_profile->GetBlotterSettings().GetActiveBlotter().GetTasksModel();
  auto tasks = std::vector<std::shared_ptr<Task>>();
  for(auto i = 0; i != tasks_model.rowCount(tasks_model.index(0, 0)); ++i) {
    auto& entry = tasks_model.GetEntry(i);
    if(!IsTerminal(entry.m_state) &&
        entry.m_state != Task::State::PENDING_CANCEL) {
      auto& task = entry.m_task;
      if(find_security(task->GetNode()) == security) {
        if(!criteria || find_price(task->GetNode()) == criteria->m_price &&
            find_destination(task->GetNode()) == criteria->m_destination) {
          tasks.push_back(task);
        }
      }
    }
  }
  execute(operation, Store(tasks));
}
