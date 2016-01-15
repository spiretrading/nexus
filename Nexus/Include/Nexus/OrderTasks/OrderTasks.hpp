#ifndef NEXUS_ORDERTASKS_HPP
#define NEXUS_ORDERTASKS_HPP

namespace Nexus {
namespace OrderTasks {
  template<typename OrderExecutionClientType> class OrderWrapperTask;
  template<typename OrderExecutionClientType> class OrderWrapperTaskFactory;
  template<typename OrderExecutionClientType> class SingleOrderTask;
  template<typename OrderExecutionClientType> class SingleOrderTaskFactory;
  class SingleRedisplayableOrderTask;
  class SingleRedisplayableOrderTaskFactory;
}
}

#endif
