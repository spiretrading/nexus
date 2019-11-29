#include "Spire/Canvas/Tasks/Executor.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Spire;

Executor::Executor()
  : m_reactor(m_producer) {}

Executor::~Executor() {
  Close();
}

void Executor::Add(Aspen::Box<void> reactor) {
  m_producer->push(Aspen::Shared(std::move(reactor)));
}

void Executor::Open() {
  if(m_openState.SetOpening()) {
    return;
  }
  m_reactorLoop = Spawn(
    [=] {
      RunLoop();
    });
  m_openState.SetOpen();
}

void Executor::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void Executor::Shutdown() {
  m_updateCondition.notify_all();
  m_openState.SetClosed();
}

void Executor::RunLoop() {
  m_has_update = false;
  auto trigger = Aspen::Trigger([=] { OnUpdate(); });
  Aspen::Trigger::set_trigger(trigger);
  auto sequence = 0;
  while(m_openState.IsRunning()) {
    auto state = m_reactor.commit(sequence);
    ++sequence;
    if(Aspen::is_complete(state)) {
      break;
    } else if(!Aspen::has_continuation(state)) {
      auto lock = std::unique_lock(m_mutex);
      while(!m_has_update && m_openState.IsRunning()) {
        m_updateCondition.wait(lock);
      }
      m_has_update = false;
    }
  }
  Close();
}

void Executor::OnUpdate() {
  {
    auto lock = std::lock_guard(m_mutex);
    m_has_update = true;
  }
  m_updateCondition.notify_one();
}
