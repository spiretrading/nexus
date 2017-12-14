#include "Spire/Canvas/TaskNodes/CanvasNodeTask.hpp"
#include <Beam/Reactors/DoReactor.hpp>
#include <Beam/Reactors/PublisherReactor.hpp>
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include "Spire/Spire/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Tasks;
using namespace boost;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace std;

CanvasNodeTask::CanvasNodeTask(RefType<CanvasNodeTranslationContext> context,
      RefType<const CanvasNode> node,
      const std::shared_ptr<SpireAggregateOrderExecutionPublisher>&
      orderExecutionPublisher)
    : m_context{Ref(context)},
      m_node{node.Get()} {
  auto translation = Translate(m_context, *m_node);
  auto& taskTranslation = get<TaskTranslation&>(translation);
  m_taskFactory = taskTranslation.m_factory;
  if(taskTranslation.m_publisher != nullptr) {
    m_publisher = taskTranslation.m_publisher;
    orderExecutionPublisher->Add(*taskTranslation.m_publisher);
  }
  m_task = m_taskFactory->Create();
}

void CanvasNodeTask::OnExecute() {
  return S0();
}

void CanvasNodeTask::OnCancel() {
  m_context.GetReactorMonitor().Do(
    [=] {
      if(m_state != 2 && m_state != 4) {
        return S5();
      }
    });
}

void CanvasNodeTask::OnTaskUpdate(const StateEntry& update) {
  m_taskState = update.m_state;
  if(m_state == 1) {
    if(m_taskState == State::ACTIVE) {
      return S3();
    } else if(m_taskState == State::FAILED) {
      return S2(update.m_message);
    }
  } else if(m_state == 3) {
    if(IsTerminal(m_taskState)) {
      return S4(update);
    }
  } else if(m_state == 5) {
    if(IsTerminal(m_taskState)) {
      return S4(update);
    }
  }
}

bool CanvasNodeTask::C0() {
  return m_taskState == State::NONE || IsTerminal(m_taskState);
}

void CanvasNodeTask::S0() {
  m_state = 0;
  return S1();
}

void CanvasNodeTask::S1() {
  m_state = 1;
  SetActive();
  m_taskState = State::INITIALIZING;
  auto publisher = MakePublisherReactor(m_task->GetPublisher());
  auto taskReactor = Reactors::Do(m_callbacks.GetCallback(
    std::bind(&CanvasNodeTask::OnTaskUpdate, this, std::placeholders::_1)),
    publisher);
  m_context.GetReactorMonitor().Add(taskReactor);
  m_task->Execute();
}

void CanvasNodeTask::S2(const string& message) {
  m_state = 2;
  SetTerminal(State::FAILED, message);
}

void CanvasNodeTask::S3() {
  m_state = 3;
}

void CanvasNodeTask::S4(StateEntry state) {
  m_state = 4;
  SetTerminal(state);
}

void CanvasNodeTask::S5() {
  m_state = 5;
  if(C0()) {
    return S4(State::CANCELED);
  }
  m_task->Cancel();
}

CanvasNodeTaskFactory::CanvasNodeTaskFactory(
    RefType<CanvasNodeTranslationContext> context,
    RefType<const CanvasNode> node)
    : m_context{context.Get()},
      m_orderExecutionPublisher{MakeAggregateOrderExecutionPublisher()},
      m_node{node.Get()} {}

std::shared_ptr<OrderExecutionPublisher> CanvasNodeTaskFactory::
    GetOrderExecutionPublisher() const {
  return m_orderExecutionPublisher;
}

std::shared_ptr<Task> CanvasNodeTaskFactory::Create() {
  return std::make_shared<CanvasNodeTask>(Ref(*m_context), Ref(*m_node),
    m_orderExecutionPublisher);
}

any& CanvasNodeTaskFactory::FindProperty(const string& name) {
  BOOST_THROW_EXCEPTION(TaskPropertyNotFoundException{name});
}

std::shared_ptr<SpireAggregateOrderExecutionPublisher>
    Spire::MakeAggregateOrderExecutionPublisher() {
  return std::make_shared<SpireAggregateOrderExecutionPublisher>(
    Initialize(UniqueFilter<const Order*>(), Initialize()));
}
