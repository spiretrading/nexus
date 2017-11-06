#ifndef NEXUS_TASKS_HPP
#define NEXUS_TASKS_HPP

namespace Nexus {
namespace Tasks {
  template<typename OrderExecutionClientType> class OrderWrapperTask;
  template<typename OrderExecutionClientType> class OrderWrapperTaskFactory;
  template<typename OrderExecutionClientType> class SingleOrderTask;
  template<typename OrderExecutionClientType> class SingleOrderTaskFactory;
  class SingleRedisplayableOrderTask;
  class SingleRedisplayableOrderTaskFactory;
}
}

#endif
