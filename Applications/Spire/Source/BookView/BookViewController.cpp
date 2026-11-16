#include "Spire/BookView/BookViewController.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

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
  if(m_window->get_current()->get()) {
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
  // TODO: Submit the task through the new active blotter.
}

void BookViewController::on_cancel_operation(
    CancelKeyBindingsModel::Operation operation, const Ticker& ticker,
    const std::unordered_map<OrderId, Money>& prices,
    const optional<std::vector<OrderId>>& ids) {
  // TODO: Cancel tasks through the new active blotter.
}
