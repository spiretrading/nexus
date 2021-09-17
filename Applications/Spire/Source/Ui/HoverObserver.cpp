#include "Spire/Ui/HoverObserver.hpp"
#include <qt_windows.h>

using namespace boost::signals2;
using namespace Spire;

QTimer HoverObserver::m_poll_timer = QTimer();

HoverObserver::HoverObserver(const QWidget& widget) {
  if(m_poll_timer.interval() == 0) {
    m_poll_timer.setInterval(10);
    QObject::connect(
      &m_poll_timer, &QTimer::timeout, [=] { on_poll_timeout(); });
    m_poll_timer.start();
  }
}

HoverObserver::State HoverObserver::get_state() const {
  return State::NONE;
}

connection HoverObserver::connect_state_signal(
    const StateSignal::slot_type& slot) const {
  return m_state_signal.connect(slot);
}

void HoverObserver::on_poll_timeout() {
  static auto count = 0;
  qDebug() << "timeout: " << ++count;
}

bool Spire::is_set(HoverObserver::State left, HoverObserver::State right) {
  return static_cast<HoverObserver::State>(
    static_cast<std::underlying_type_t<HoverObserver::State>>(left) &
    static_cast<std::underlying_type_t<HoverObserver::State>>(right)) !=
    HoverObserver::State::NONE;
}
