#ifndef NEXUS_SIMULATION_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_SIMULATION_ORDER_EXECUTION_DRIVER_HPP
#include <unordered_map>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderUnrecoverableException.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/SimulationMatcher/SecurityOrderSimulator.hpp"
#include "Nexus/SimulationMatcher/SimulationMatcher.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * An OrderExecutionDriver that simulates transactions.
   * @param C The type of MarketDataClient to use.
   * @param T The type of TimeClient used for Order timestamps.
   */
  template<typename C, typename T>
  class SimulationOrderExecutionDriver {
    public:

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::GetTryDereferenceType<C>;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /**
       * Constructs a SimulationOrderExecutionDriver.
       * @param marketDataClient Initializes the MarketDataClient.
       * @param timeClient Initializes the TimeClient.
       */
      template<typename CF, typename TF>
      SimulationOrderExecutionDriver(CF&& marketDataClient, TF&& timeClient);

      ~SimulationOrderExecutionDriver();

      const Order& Recover(const SequencedAccountOrderRecord& orderRecord);

      const Order& Submit(const OrderInfo& orderInfo);

      void Cancel(const OrderExecutionSession& session, OrderId orderId);

      void Update(const OrderExecutionSession& session, OrderId orderId,
        const ExecutionReport& executionReport);

      void Close();

    private:
      using SecurityOrderSimulator =
        OrderExecutionService::SecurityOrderSimulator<TimeClient>;
      using Orders =
        std::unordered_map<OrderId, std::shared_ptr<PrimitiveOrder>>;
      using SecurityOrderSimulators = std::unordered_map<Security,
        std::unique_ptr<SecurityOrderSimulator>>;
      Beam::GetOptionalLocalPtr<C> m_marketDataClient;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      Beam::SynchronizedMap<Orders> m_orders;
      OrderId m_nextOrderId;
      Beam::SynchronizedMap<SecurityOrderSimulators, Beam::Threading::Mutex>
        m_securityOrderSimulators;
      Beam::IO::OpenState m_openState;

      SimulationOrderExecutionDriver(
        const SimulationOrderExecutionDriver&) = delete;
      SimulationOrderExecutionDriver& operator =(
        const SimulationOrderExecutionDriver&) = delete;
      SecurityOrderSimulator& LoadSimulator(const Security& security);
  };

  template<typename C, typename T>
  template<typename CF, typename TF>
  SimulationOrderExecutionDriver<C, T>::SimulationOrderExecutionDriver(
    CF&& marketDataClient, TF&& timeClient)
    : m_marketDataClient(std::forward<CF>(marketDataClient)),
      m_timeClient(std::forward<TF>(timeClient)),
      m_nextOrderId(1) {}

  template<typename C, typename T>
  SimulationOrderExecutionDriver<C, T>::~SimulationOrderExecutionDriver() {
    Close();
  }

  template<typename C, typename T>
  const Order& SimulationOrderExecutionDriver<C, T>::Recover(
      const SequencedAccountOrderRecord& orderRecord) {
    auto order = std::make_shared<PrimitiveOrder>(**orderRecord);
    m_orders.Insert((*orderRecord)->m_info.m_orderId, order);
    auto& simulator = LoadSimulator((*orderRecord)->m_info.m_fields.m_security);
    simulator.Recover(order);
    return *order;
  }

  template<typename C, typename T>
  const Order& SimulationOrderExecutionDriver<C, T>::Submit(
      const OrderInfo& orderInfo) {
    auto order = std::make_shared<PrimitiveOrder>(orderInfo);
    m_orders.Insert(orderInfo.m_orderId, order);
    auto& simulator = LoadSimulator(orderInfo.m_fields.m_security);
    simulator.Submit(order);
    return *order;
  }

  template<typename C, typename T>
  void SimulationOrderExecutionDriver<C, T>::Cancel(
      const OrderExecutionSession& session, OrderId orderId) {
    if(auto order = m_orders.Find(orderId)) {
      auto& simulator = LoadSimulator((*order)->GetInfo().m_fields.m_security);
      simulator.Cancel(*order);
    }
  }

  template<typename C, typename T>
  void SimulationOrderExecutionDriver<C, T>::Update(
      const OrderExecutionSession& session, OrderId orderId,
      const ExecutionReport& executionReport) {
    if(auto order = m_orders.Find(orderId)) {
      auto& simulator = LoadSimulator((*order)->GetInfo().m_fields.m_security);
      simulator.Update(*order, executionReport);
    }
  }

  template<typename C, typename T>
  void SimulationOrderExecutionDriver<C, T>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_securityOrderSimulators.Clear();
    m_openState.Close();
  }

  template<typename C, typename T>
  typename SimulationOrderExecutionDriver<C, T>::SecurityOrderSimulator&
      SimulationOrderExecutionDriver<C, T>::LoadSimulator(
      const Security& security) {
    return *m_securityOrderSimulators.GetOrInsert(security, [&] {
      return std::make_unique<SecurityOrderSimulator>(*m_marketDataClient,
        security, Beam::Ref(*m_timeClient));
    });
  }
}

#endif
