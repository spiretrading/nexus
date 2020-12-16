#ifndef NEXUS_ORDER_EXECUTION_SERVICE_HPP
#define NEXUS_ORDER_EXECUTION_SERVICE_HPP
#include <cstdint>
#include <string>
#include <vector>
#include <Beam/Queues/Queues.hpp>

namespace Nexus::OrderExecutionService {
  class AccountOrderSubmissionEntry;
  template<typename C> class BoardLotCheck;
  template<typename A, typename M> class BuyingPowerCheck;
  struct ExecutionReport;
  struct ExecutionReportEntry;
  class ExecutionReportPublisher;
  class LocalOrderExecutionDataStore;
  template<typename D, typename A> class ManualOrderEntryDriver;
  class Order;
  template<typename C, typename S> class OrderCancellationReactor;
  using OrderId = std::uint64_t;
  template<typename B> class OrderExecutionClient;
  class OrderExecutionClientBox;
  struct OrderExecutionDataStore;
  class OrderExecutionDataStoreBox;
  class OrderExecutionDataStoreException;
  using OrderExecutionPublisher = Beam::SnapshotPublisher<const Order*,
    std::vector<const Order*>>;
  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> class OrderExecutionServlet;
  class OrderExecutionSession;
  struct OrderFields;
  struct OrderInfo;
  template<typename C, typename AR, typename SR, typename CR, typename OR,
    typename TR, typename DR, typename QR, typename PR, typename FR,
    typename RR> class OrderReactor;
  struct OrderRecord;
  class OrderSubmissionCheck;
  template<typename OrderExecutionDriverType> class OrderSubmissionCheckDriver;
  class OrderSubmissionCheckException;
  class OrderSubmissionRegistry;
  class OrderUnrecoverableException;
  class OrderWrapperReactor;
  class PrimitiveOrder;
  template<typename AdministrationClientType> class RiskStateCheck;
  template<typename C> class SqlOrderExecutionDataStore;
  class VirtualOrderExecutionDriver;
  template<typename DriverType> class WrapperOrderExecutionDriver;

  /** Standard name for the order execution service. */
  inline const std::string SERVICE_NAME = "order_execution_service";
}

#endif
