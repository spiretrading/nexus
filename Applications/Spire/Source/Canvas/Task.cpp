#include "Spire/Canvas/Task.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Spire;

Task::StateEntry::StateEntry(State state)
  : m_state(state) {}

Task::StateEntry::StateEntry(State state, std::string message)
  : m_state(state),
    m_message(std::move(message)) {}

Task::Task(int id, Box<void> reactor)
  : m_id(id),
    m_is_executable(true),
    m_is_cancelable(true),
    m_is_failed(false),
    m_state(State::READY),
    m_reactor(std::move(reactor)) {}

int Task::get_id() const {
  return m_id;
}

void Task::execute() {
  if(!std::exchange(m_is_executable, false)) {
    return;
  }
  m_executor.emplace(
    box(
      chain(
        lift([=] {
          m_state = State::INITIALIZING;
          m_publisher.Push(StateEntry(m_state));
        }),
        until(m_cancel_token,
          lift([=] (const Maybe<void>& value) {
            if(m_state == State::INITIALIZING) {
              m_state = State::ACTIVE;
              m_publisher.Push(StateEntry(m_state));
            }
            try {
              value.get();
            } catch(...) {
              m_is_failed = true;
              m_cancel_token->set_complete(true);
            }
          }, std::move(m_reactor))),
        lift([=] {
          if(!m_is_cancelable) {
            m_state = State::PENDING_CANCEL;
            m_publisher.Push(StateEntry(m_state));
          }
        }),
        lift([=] {
          if(m_is_failed) {
            m_state = State::FAILED;
          } else if(m_state == State::PENDING_CANCEL) {
            m_state = State::CANCELED;
          } else {
            m_state = State::COMPLETE;
          }
          m_publisher.Push(StateEntry(m_state));
        }))));
}

void Task::cancel() {
  if(std::exchange(m_is_executable, false)) {
    m_publisher.Push(StateEntry(State::PENDING_CANCEL));
    m_state = State::CANCELED;
    m_publisher.Push(StateEntry(m_state, "Canceled by user."));
  } else if(m_is_cancelable.exchange(false)) {
    m_cancel_token->set_complete(true);
  }
}

const Publisher<Task::StateEntry>& Task::get_publisher() const {
  return m_publisher;
}

std::ostream& Spire::operator <<(std::ostream& out, Task::State state) {
  if(state == Task::State::READY) {
    return out << "READY";
  } else if(state == Task::State::INITIALIZING) {
    return out << "INITIALIZING";
  } else if(state == Task::State::ACTIVE) {
    return out << "ACTIVE";
  } else if(state == Task::State::PENDING_CANCEL) {
    return out << "PENDING_CANCEL";
  } else if(state == Task::State::CANCELED) {
    return out << "CANCELED";
  } else if(state == Task::State::FAILED) {
    return out << "FAILED";
  } else if(state == Task::State::COMPLETE) {
    return out << "COMPLETE";
  }
  return out;
}

bool Spire::is_terminal(Task::State state) {
  return state == Task::State::CANCELED ||
    state == Task::State::FAILED || state == Task::State::COMPLETE;
}
