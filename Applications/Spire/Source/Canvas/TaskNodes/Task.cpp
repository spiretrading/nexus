#include "Spire/Canvas/TaskNodes/Task.hpp"
#include <atomic>

using namespace Aspen;
using namespace Beam;
using namespace Spire;

namespace {
  std::atomic_int nextId = 0;
}

Task::StateEntry::StateEntry(State state)
  : m_state(state) {}

Task::StateEntry::StateEntry(State::Type state)
  : m_state(state) {}

Task::StateEntry::StateEntry(State state, const std::string& message)
  : m_state(state),
    m_message(message) {}

Task::Task(Box<void> reactor)
  : m_reactor(std::move(reactor)),
    m_id(++nextId),
    m_publisher(std::make_unique<SequencePublisher<StateEntry>>()) {}

std::int32_t Task::GetId() const {
  return m_id;
}

void Task::Execute() {}

void Task::Cancel() {}

const Publisher<Task::StateEntry>& Task::GetPublisher() const {
  return *m_publisher;
}

State Task::commit(int sequence) noexcept {
  return Aspen::State::NONE;
}

const Task::StateEntry& Task::eval() const noexcept {
  static auto value = StateEntry();
  return value;
}

std::ostream& Spire::operator <<(std::ostream& s, Task::State state) {
  if(state == Task::State::NONE) {
    s << "None";
  } else if(state == Task::State::INITIALIZING) {
    s << "Initializing";
  } else if(state == Task::State::ACTIVE) {
    s << "Active";
  } else if(state == Task::State::PENDING_CANCEL) {
    s << "Pending Cancel";
  } else if(state == Task::State::CANCELED) {
    s << "Canceled";
  } else if(state == Task::State::FAILED) {
    s << "Failed";
  } else if(state == Task::State::EXPIRED) {
    s << "Expired";
  } else if(state == Task::State::COMPLETE) {
    s << "Complete";
  }
  return s;
}

bool Spire::IsTerminal(Task::State state) {
  return state == Task::State::CANCELED || state == Task::State::FAILED ||
    state == Task::State::EXPIRED || state == Task::State::COMPLETE;
}
