#include "Spire/BookView/BookViewController.hpp"
#include <algorithm>
#include <QMessageBox>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/BookView/ServiceBookViewModel.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include "Spire/Canvas/Operations/FindCanvasNodeValue.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  optional<Ticker> find_ticker(const CanvasNode& node) {
    return find_value<TickerNode>(node, SingleOrderTaskNode::TICKER_PROPERTY);
  }

  bool is_match(Task& task, const std::vector<OrderId>& ids) {
    auto is_found = false;
    task.GetContext().GetOrderPublisher().with([&] (auto orders) {
      if(!orders) {
        return;
      }
      for(auto& order : *orders) {
        if(std::ranges::find(ids, order->get_info().m_id) != ids.end()) {
          is_found = true;
          return;
        }
      }
    });
    return is_found;
  }
}

struct BookViewController::EventFilter : QObject {
  BookViewController* m_controller;

  EventFilter(BookViewController& controller)
    : m_controller(&controller) {}

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::Close) {
      m_controller->close();
      return false;
    }
    return QObject::eventFilter(watched, event);
  }
};

BookViewController::BookViewController(Ref<UserProfile> user_profile)
  : m_user_profile(user_profile.get()),
    m_window(nullptr) {}

BookViewController::BookViewController(
    Ref<UserProfile> user_profile, BookViewWindow& window)
    : m_user_profile(user_profile.get()) {
  set_window(window);
}

BookViewController::~BookViewController() {
  close();
  if(m_event_filter) {
    m_event_filter.release()->deleteLater();
  }
}

void BookViewController::open() {
  if(m_window) {
    m_window->show();
    return;
  }
  auto window = new BookViewWindow(Ref(*m_user_profile),
    m_user_profile->GetTickerInfoQueryModel(),
    m_user_profile->GetKeyBindings(),
    m_user_profile->GetBookViewPropertiesWindowFactory(),
    m_user_profile->GetBookViewModelBuilder());
  set_window(*window);
  window->show();
}

void BookViewController::close() {
  if(!m_window) {
    return;
  }
  if(m_window->GetDisplayedTicker()) {
    m_user_profile->GetRecentlyClosedWindows()->push(
      m_window->GetWindowSettings());
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

void BookViewController::set_window(BookViewWindow& window) {
  m_event_filter = std::make_unique<EventFilter>(*this);
  m_window = &window;
  m_window->installEventFilter(m_event_filter.get());
  m_submit_task_connection = m_window->connect_submit_task_signal(
    std::bind_front(&BookViewController::on_submit_task, this));
  m_cancel_operation_connection = m_window->connect_cancel_operation_signal(
    std::bind_front(&BookViewController::on_cancel_operation, this));
}

void BookViewController::on_submit_task(
    const std::shared_ptr<CanvasNode>& task) {
  auto errors = Validate(*task);
  if(!errors.empty()) {
    QMessageBox::warning(m_window, QObject::tr("Error"),
      QString::fromStdString(errors.front().GetErrorMessage()));
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
    CancelKeyBindingsModel::Operation operation, const Ticker& ticker,
    const optional<std::vector<OrderId>>& ids) {
  auto& tasks_model =
    m_user_profile->GetBlotterSettings().GetActiveBlotter().GetTasksModel();
  auto tasks = std::vector<std::shared_ptr<Task>>();
  for(auto i = 0; i != tasks_model.rowCount(tasks_model.index(0, 0)); ++i) {
    auto& entry = tasks_model.GetEntry(i);
    if(IsTerminal(entry.m_state) ||
        entry.m_state == Task::State::PENDING_CANCEL) {
      continue;
    }
    auto& task = entry.m_task;
    auto& node = task->GetNode();
    if(find_ticker(node) != ticker) {
      continue;
    }
    if(!ids || is_match(*task, *ids)) {
      tasks.push_back(task);
    }
  }
  execute(operation, out(tasks));
}
