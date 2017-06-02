#ifndef NEXUS_ORDEREXECUTIONSERVICE_HPP
#define NEXUS_ORDEREXECUTIONSERVICE_HPP
#include <cstdint>
#include <string>
#include <vector>
#include <Beam/Queues/Queues.hpp>

namespace Nexus {
namespace OrderExecutionService {
  class AccountOrderSubmissionEntry;
  class ApplicationOrderExecutionClient;
  template<typename MarketDataClientType> class BoardLotCheck;
  template<typename AdministrationClientType, typename MarketDataClientType>
    class BuyingPowerCheck;
  struct ExecutionReport;
  struct ExecutionReportEntry;
  class ExecutionReportPublisher;
  class LocalOrderExecutionDataStore;
  template<typename OrderExecutionDriverType, typename AdministrationClientType>
    class ManualOrderEntryDriver;
  class MySqlOrderExecutionDataStore;
  class Order;
  typedef std::uint64_t OrderId;
  template<typename ServiceProtocolClientBuilderType>
    class OrderExecutionClient;
  struct OrderExecutionDataStore;
  class OrderExecutionDataStoreException;
  typedef Beam::SnapshotPublisher<const Order*, std::vector<const Order*>>
    OrderExecutionPublisher;
  template<typename ContainerType, typename TimeClientType,
    typename ServiceLocatorClientType, typename UidClientType,
    typename AdministrationClientType, typename OrderExecutionDriverType,
    typename OrderExecutionDataStoreType> class OrderExecutionServlet;
  class OrderExecutionSession;
  struct OrderFields;
  struct OrderInfo;
  struct OrderRecord;
  class OrderSubmissionCheck;
  template<typename OrderExecutionDriverType> class OrderSubmissionCheckDriver;
  class OrderSubmissionCheckException;
  class OrderSubmissionRegistry;
  class OrderUnrecoverableException;
  class PrimitiveOrder;
  template<typename AdministrationClientType> class RiskStateCheck;
  class VirtualOrderExecutionClient;
  class VirtualOrderExecutionDataStore;
  class VirtualOrderExecutionDriver;
  template<typename ClientType> class WrapperOrderExecutionClient;
  template<typename DataStoreType> class WrapperOrderExecutionDataStore;
  template<typename DriverType> class WrapperOrderExecutionDriver;

  // Standard name for the order execution service.
  static const std::string SERVICE_NAME = "order_execution_service";
}
}

#endif
