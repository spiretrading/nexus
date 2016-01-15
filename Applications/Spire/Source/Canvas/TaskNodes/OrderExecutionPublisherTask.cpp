#include "Spire/Canvas/TaskNodes/OrderExecutionPublisherTask.hpp"

using namespace Beam;
using namespace Beam::Tasks;
using namespace boost;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace std;

OrderExecutionPublisherTaskFactory::OrderExecutionPublisherTaskFactory(
    const TaskFactory& taskFactory,
    std::shared_ptr<OrderExecutionPublisher> publisher)
    : m_taskFactory(taskFactory),
      m_publisher(publisher) {}

std::shared_ptr<Task> OrderExecutionPublisherTaskFactory::Create() {
  return m_taskFactory->Create();
}

any& OrderExecutionPublisherTaskFactory::FindProperty(const string& name) {
  return m_taskFactory->FindProperty(name);
}

void OrderExecutionPublisherTaskFactory::PrepareContinuation(const Task& task) {
  m_taskFactory->PrepareContinuation(task);
}
