#ifndef NEXUS_ORDER_HPP
#define NEXUS_ORDER_HPP
#include <Beam/Queues/SnapshotPublisher.hpp>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderInfo.hpp"

namespace Nexus::OrderExecutionService {

  /** Interface for keeping track of the state of an Order. */
  class Order {
    public:
      virtual ~Order() = default;

      /** Returns the submission info. */
      virtual const OrderInfo& GetInfo() const = 0;

      /** Returns the object publishing ExecutionReport updates. */
      virtual const Beam::SnapshotPublisher<ExecutionReport,
        std::vector<ExecutionReport>>& GetPublisher() const = 0;

    protected:

      /** Constructs an Order. */
      Order() = default;

    private:
      Order(const Order&) = delete;
      Order& operator =(const Order&) = delete;
  };
}

#endif
