#include "Spire/Ui/ClickObserver.hpp"
#include <QWidget>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

ClickObserver::ClickObserver(const QWidget& widget)
    : m_widget(&widget),
      m_press_observer(widget) {
  m_press_observer.connect_press_end_signal(
    std::bind_front(&ClickObserver::on_press_end, this));
}

connection ClickObserver::connect_click_signal(
    const ClickSignal::slot_type& slot) const {
  return m_click_signal.connect(slot);
}

void ClickObserver::on_press_end(PressObserver::Reason reason) {
  if(reason == PressObserver::Reason::KEYBOARD ||
      m_widget->rect().contains(m_widget->mapFromGlobal(QCursor::pos()))) {
    m_click_signal();
  }
}
