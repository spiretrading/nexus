#include "Spire/Canvas/Executor.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Spire;

Executor::Executor(Aspen::Box<void> reactor)
    : m_trigger(std::bind_front(&Executor::on_update, this)),
      m_reactor(std::move(reactor)) {
  m_reactor_loop = Spawn(std::bind_front(&Executor::run, this));
}

Executor::~Executor() {
  m_open_state.Close();
  m_update_condition.notify_all();
}

void Executor::run() {
  m_has_update = false;
  auto sequence = 0;
  while(m_open_state.IsOpen()) {
    Aspen::Trigger::set_trigger(m_trigger);
    auto state = m_reactor.commit(sequence);
    ++sequence;
    if(Aspen::is_complete(state)) {
      break;
    } else if(!Aspen::has_continuation(state)) {
      auto lock = std::unique_lock(m_mutex);
      while(!m_has_update && m_open_state.IsOpen()) {
        m_update_condition.wait(lock);
      }
      m_has_update = false;
    }
  }
  m_open_state.Close();
  m_update_condition.notify_all();
}

void Executor::on_update() {
  {
    auto lock = std::lock_guard(m_mutex);
    m_has_update = true;
  }
  m_update_condition.notify_all();
}
