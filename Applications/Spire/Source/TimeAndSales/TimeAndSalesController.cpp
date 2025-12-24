#include "Spire/TimeAndSales/TimeAndSalesController.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Spire;

struct TimeAndSalesController::EventFilter : QObject {
  TimeAndSalesController* m_controller;

  EventFilter(TimeAndSalesController& controller)
    : m_controller(&controller) {}

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::Close) {
      m_controller->close();
    }
    return QObject::eventFilter(watched, event);
  }
};

TimeAndSalesController::TimeAndSalesController(Ref<UserProfile> user_profile)
  : m_user_profile(user_profile.get()),
    m_window(nullptr) {}

TimeAndSalesController::TimeAndSalesController(
    Ref<UserProfile> user_profile, TimeAndSalesWindow& window)
    : m_user_profile(user_profile.get()) {
  set_window(window);
}

TimeAndSalesController::~TimeAndSalesController() {
  close();
}

void TimeAndSalesController::open() {
  if(m_window) {
    m_window->show();
    return;
  }
  auto window = new TimeAndSalesWindow(
    m_user_profile->GetSecurityInfoQueryModel(),
    m_user_profile->GetTimeAndSalesPropertiesWindowFactory(),
    m_user_profile->GetTimeAndSalesModelBuilder());
  set_window(*window);
  window->show();
}

void TimeAndSalesController::close() {
  if(!m_window) {
    return;
  }
  if(m_window->get_current()) {
    m_user_profile->GetRecentlyClosedWindows()->push(
      m_window->GetWindowSettings());
  }
  m_window->removeEventFilter(m_event_filter.get());
  m_window->close();
  m_window->deleteLater();
  m_window = nullptr;
  m_closed_signal();
}

connection TimeAndSalesController::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void TimeAndSalesController::set_window(TimeAndSalesWindow& window) {
  m_event_filter = std::make_unique<EventFilter>(*this);
  m_window = &window;
  m_window->installEventFilter(m_event_filter.get());
}
