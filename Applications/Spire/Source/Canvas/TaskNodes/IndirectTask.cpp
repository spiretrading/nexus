#include "Spire/Canvas/TaskNodes/IndirectTask.hpp"
#include <Beam/Reactors/DoReactor.hpp>
#include <Beam/Reactors/QueueReactor.hpp>
#include <Beam/Reactors/ReactorMonitor.hpp>
#include <boost/throw_exception.hpp>

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Tasks;
using namespace boost;
using namespace Spire;
using namespace std;

IndirectTask::IndirectTask(RefType<ReactorMonitor> reactorMonitor)
    : m_reactorMonitor{reactorMonitor.Get()} {}

void IndirectTask::SetTask(std::shared_ptr<Task> task) {
  m_task = std::move(task);
}

void IndirectTask::OnExecute() {
  assert(m_task != nullptr);
  auto publisher = MakePublisherReactor(m_task->GetPublisher(),
    Ref(m_reactorMonitor->GetTrigger()));
  auto taskReactor = Do(m_callbacks.GetCallback(
    std::bind(&IndirectTask::OnTaskUpdate, this, std::placeholders::_1)),
    publisher);
  m_reactorMonitor->Add(taskReactor);
  m_task->Execute();
}

void IndirectTask::OnCancel() {
  m_reactorMonitor->Do(
    [=] {
      if(m_task == nullptr) {
        return;
      }
      m_task->Cancel();
    });
}

void IndirectTask::OnTaskUpdate(const StateEntry& update) {
  if(update.m_state == State::ACTIVE) {
    SetActive(update.m_message);
  } else if(IsTerminal(update.m_state)) {
    m_task.reset();
    SetTerminal(update.m_state, update.m_message);
  }
}

IndirectTaskFactory::IndirectTaskFactory(RefType<ReactorMonitor> reactorMonitor)
    : m_taskFactory{std::make_shared<optional<TaskFactory>>()},
      m_task{std::make_shared<IndirectTask>(Ref(reactorMonitor))} {}

const std::shared_ptr<IndirectTask>& IndirectTaskFactory::GetTask() const {
  return m_task;
}

void IndirectTaskFactory::SetTaskFactory(const TaskFactory& taskFactory) {
  assert(!m_taskFactory->is_initialized());
  *m_taskFactory = taskFactory;
}

std::shared_ptr<Task> IndirectTaskFactory::Create() {
  m_task->SetTask((**m_taskFactory)->Create());
  return std::move(m_task);
}

any& IndirectTaskFactory::FindProperty(const string& name) {
  if(m_taskFactory->is_initialized()) {
    return (**m_taskFactory)->FindProperty(name);
  }
  BOOST_THROW_EXCEPTION(TaskPropertyNotFoundException{name});
}
