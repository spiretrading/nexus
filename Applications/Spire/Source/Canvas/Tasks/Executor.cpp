#include "Spire/Canvas/Tasks/Executor.hpp"
#include <Beam/Utilities/BeamWorkaround.hpp>

using namespace Beam;
using namespace Beam::Routines;
using namespace Spire;

Executor::Executor()
BEAM_SUPPRESS_THIS_INITIALIZER()
  : m_trigger([this] { OnUpdate(); }),
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    m_reactor(m_producer) {}

Executor::~Executor() {
  Close();
}

void Executor::Add(Aspen::Box<void> reactor) {
  m_producer->push(Aspen::Shared(std::move(reactor)));
}

void Executor::Open() {
  m_reactorLoop = Spawn([=] {
    RunLoop();
  });
}

void Executor::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  m_updateCondition.notify_all();
  m_openState.Close();
}

void Executor::RunLoop() {
  m_has_update = false;
  auto sequence = 0;
  while(m_openState.IsOpen()) {
    Aspen::Trigger::set_trigger(m_trigger);
    auto state = m_reactor.commit(sequence);
    ++sequence;
    if(Aspen::is_complete(state)) {
      break;
    } else if(!Aspen::has_continuation(state)) {
      auto lock = std::unique_lock(m_mutex);
      while(!m_has_update && m_openState.IsOpen()) {
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
