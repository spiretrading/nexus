#include "Spire/Spire/EventHandler.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QCoreApplication>

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Spire;

namespace {
  constexpr auto UPDATE_INTERVAL = 100;
}

EventHandler::EventHandler()
    : m_tasks(std::make_shared<TaskQueue>()) {
  connect(&m_update_timer, &QTimer::timeout, this, &EventHandler::on_expired);
  m_update_timer.start(UPDATE_INTERVAL);
}

void EventHandler::on_expired() {
  auto start = microsec_clock::universal_time();
  auto tasks = m_tasks;
  while(tasks.use_count() != 1 && !tasks->IsEmpty()) {
    std::function<void ()> task;
    tasks->Emplace(Store(task));
    task();
    auto duration = microsec_clock::universal_time();
    if(duration - start > seconds(1) / 10) {
      QCoreApplication::instance()->processEvents();
      start = microsec_clock::universal_time();
    }
  }
}
