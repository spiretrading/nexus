#include "Spire/Canvas/Task.hpp"

using namespace Beam;
using namespace Spire;

namespace {
  auto next_id = 0;
}

Task::StateEntry::StateEntry(State state)
  : m_state(state) {}

Task::StateEntry::StateEntry(State state, std::string message)
  : m_state(state),
    m_message(std::move(message)) {}

Task::Task()
  : m_id(++next_id) {}

int Task::get_id() const {
  return m_id;
}

void Task::execute() {}

void Task::cancel() {}

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
  return state == Task::State::CANCELED || state == Task::State::FAILED ||
    state == Task::State::COMPLETE;
}
