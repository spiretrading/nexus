#include "Spire/Canvas/TaskNodes/ReactorMonitor.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Routines;
using namespace Spire;

ReactorMonitor::~ReactorMonitor() {
  Close();
}

void ReactorMonitor::Add(Aspen::Box<void> reactor) {
  m_producer.push(std::move(reactor));
}

void ReactorMonitor::Wait() {
  auto lock = std::unique_lock(m_mutex);
  while(m_openState.IsRunning()) {
    m_updateCondition.wait(lock);
  }
}

void ReactorMonitor::Open() {
  if(m_openState.SetOpening()) {
    return;
  }
  m_reactorLoop = Spawn(
    [=] {
      RunLoop();
    });
  m_openState.SetOpen();
}

void ReactorMonitor::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void ReactorMonitor::Shutdown() {
  m_updateCondition.notify_all();
  m_openState.SetClosed();
}

void ReactorMonitor::RunLoop() {
  m_has_update = false;
  auto trigger = Trigger([=] { OnUpdate(); });
  Trigger::set_trigger(trigger);
  auto sequence = 0;
  while(m_openState.IsRunning()) {
    auto state = m_reactor.commit(sequence);
    ++sequence;
    if(is_complete(state)) {
      break;
    } else if(!has_continuation(state)) {
      auto lock = std::unique_lock(m_mutex);
      while(!m_has_update && m_openState.IsRunning()) {
        m_updateCondition.wait(lock);
      }
      m_has_update = false;
    }
  }
}

void ReactorMonitor::OnUpdate() {
  {
    auto lock = std::lock_guard(m_mutex);
    m_has_update = true;
  }
  m_updateCondition.notify_one();
}
