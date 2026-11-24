#include "Spire/Canvas/Tasks/Executor.hpp"
#include <condition_variable>
#include <mutex>
#include <Aspen/CommitFlag.hpp>
#include <Aspen/Concur.hpp>
#include <Aspen/Queue.hpp>
#include <Aspen/Shared.hpp>
#include <Aspen/Trigger.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Routines/Scheduler.hpp>

using namespace Aspen;
using namespace Beam;
using namespace Spire;

struct Executor::State {
  mutable std::mutex m_mutex;
  std::condition_variable m_condition;
  OpenState m_open_state;
  Trigger m_trigger;
  CommitFlag m_flag;
  bool m_has_update;
  Shared<Queue<SharedBox<void>>> m_producer;
  Concur<Shared<Queue<SharedBox<void>>>> m_reactor;

  State();

  Aspen::State commit(int sequence);
  void run();
  void close();
  void on_update();
};

Executor::Executor()
  : m_state(std::make_shared<State>()) {}

Executor::~Executor() {
  Close();
}

void Executor::Add(Box<void> reactor) {
  m_state->m_producer->push(Shared(std::move(reactor)));
}

void Executor::Open() {
  Open(std::shared_ptr<void>());
}

void Executor::Close() {
  m_state->close();
}

void Executor::Open(std::shared_ptr<void> lifetime) {
  spawn([state = m_state, lifetime = std::move(lifetime)] {
    state->run();
  });
}

Executor::State::State()
    : m_trigger([=, this] {
        on_update();
      }),
      m_has_update(false),
      m_reactor(m_producer) {
  m_flag.set_trigger(&m_trigger);
}

Aspen::State Executor::State::commit(int sequence) {
  m_flag.clear();
  auto scope = CommitFlagScope(m_flag);
  auto previous = Trigger::get_trigger();
  Trigger::set_trigger(m_trigger);
  auto state = m_reactor.commit(sequence);
  Trigger::set_trigger(previous);
  return state;
}

void Executor::State::run() {
  auto sequence = 0;
  while(m_open_state.is_open()) {
    auto state = commit(sequence);
    ++sequence;
    if(is_complete(state)) {
      break;
    } else if(!has_continuation(state)) {
      auto lock = std::unique_lock(m_mutex);
      while(!m_has_update && m_open_state.is_open()) {
        m_condition.wait(lock);
      }
      m_has_update = false;
    }
  }
  close();
}

void Executor::State::close() {
  if(m_open_state.set_closing()) {
    return;
  }
  {
    auto lock = std::lock_guard(m_mutex);
    m_condition.notify_all();
  }
  m_open_state.close();
}

void Executor::State::on_update() {
  {
    auto lock = std::lock_guard(m_mutex);
    m_has_update = true;
  }
  m_condition.notify_one();
}
