#ifndef NEXUS_SIMULATIONORDEREXECUTIONDRIVER_HPP
#define NEXUS_SIMULATIONORDEREXECUTIONDRIVER_HPP
#include <unordered_map>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderUnrecoverableException.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "SimulationOrderExecutionServer/SecurityOrderSimulator.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class SimulationOrderExecutionDriver
      \brief An OrderExecutionDriver that simulates transactions.
      \tparam MarketDataClientType The type of MarketDataClient to use.
      \tparam TimeClientType The type of TimeClient used for Order timestamps.
   */
  template<typename MarketDataClientType, typename TimeClientType>
  class SimulationOrderExecutionDriver : private boost::noncopyable {
    public:

      //! The type of MarketDataClient to use.
      using MarketDataClient =
        Beam::GetTryDereferenceType<MarketDataClientType>;

      //! The type of TimeClient to use.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! Constructs a SimulationOrderExecutionDriver.
      /*!
        \param marketDataClient Initializes the MarketDataClient.
        \param timeClient Initializes the TimeClient.
      */
      template<typename MarketDataClientForward, typename TimeClientForward>
      SimulationOrderExecutionDriver(MarketDataClientForward&& marketDataClient,
        TimeClientForward&& timeClient);

      ~SimulationOrderExecutionDriver();

      const Order& Recover(const SequencedAccountOrderRecord& orderRecord);

      const Order& Submit(const OrderInfo& orderInfo);

      void Cancel(const OrderExecutionSession& session, OrderId orderId);

      void Update(const OrderExecutionSession& session, OrderId orderId,
        const ExecutionReport& executionReport);

      void Open();

      void Close();

    private:
      using SecurityOrderSimulator =
        OrderExecutionService::SecurityOrderSimulator<TimeClient>;
      using Orders =
        std::unordered_map<OrderId, std::shared_ptr<PrimitiveOrder>>;
      using SecurityOrderSimulators = std::unordered_map<Security,
        std::shared_ptr<SecurityOrderSimulator>>;
      Beam::GetOptionalLocalPtr<MarketDataClientType> m_marketDataClient;
      Beam::GetOptionalLocalPtr<TimeClientType> m_timeClient;
      Beam::SynchronizedMap<Orders> m_orders;
      OrderId m_nextOrderId;
      Beam::SynchronizedMap<SecurityOrderSimulators> m_securityOrderSimulators;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      SecurityOrderSimulator& LoadSimulator(const Security& security);
  };

  template<typename MarketDataClientType, typename TimeClientType>
  template<typename MarketDataClientForward, typename TimeClientForward>
  SimulationOrderExecutionDriver<MarketDataClientType, TimeClientType>::
      SimulationOrderExecutionDriver(MarketDataClientForward&& marketDataClient,
      TimeClientForward&& timeClient)
      : m_marketDataClient(std::forward<MarketDataClientForward>(
          marketDataClient)),
        m_timeClient(std::forward<TimeClientForward>(timeClient)),
        m_nextOrderId(1) {}

  template<typename MarketDataClientType, typename TimeClientType>
  SimulationOrderExecutionDriver<MarketDataClientType, TimeClientType>::
      ~SimulationOrderExecutionDriver() {
    Close();
  }

  template<typename MarketDataClientType, typename TimeClientType>
  const Order& SimulationOrderExecutionDriver<MarketDataClientType,
      TimeClientType>::Recover(const SequencedAccountOrderRecord& orderRecord) {
    auto order = std::make_shared<PrimitiveOrder>(**orderRecord);
    m_orders.Insert((*orderRecord)->m_info.m_orderId, order);
    auto& simulator = LoadSimulator((*orderRecord)->m_info.m_fields.m_security);
    simulator.Recover(order);
    return *order;
  }

  template<typename MarketDataClientType, typename TimeClientType>
  const Order& SimulationOrderExecutionDriver<MarketDataClientType,
      TimeClientType>::Submit(const OrderInfo& orderInfo) {
    auto order = std::make_shared<PrimitiveOrder>(orderInfo);
    m_orders.Insert(orderInfo.m_orderId, order);
    auto& simulator = LoadSimulator(orderInfo.m_fields.m_security);
    simulator.Submit(order);
    return *order;
  }

  template<typename MarketDataClientType, typename TimeClientType>
  void SimulationOrderExecutionDriver<MarketDataClientType, TimeClientType>::
      Cancel(const OrderExecutionSession& session, OrderId orderId) {
    auto order = m_orders.Find(orderId);
    if(!order.is_initialized()) {
      return;
    }
    auto& simulator = LoadSimulator((*order)->GetInfo().m_fields.m_security);
    simulator.Cancel(*order);
  }

  template<typename MarketDataClientType, typename TimeClientType>
  void SimulationOrderExecutionDriver<MarketDataClientType, TimeClientType>::
      Update(const OrderExecutionSession& session, OrderId orderId,
      const ExecutionReport& executionReport) {
    auto order = m_orders.Find(orderId);
    if(!order.is_initialized()) {
      return;
    }
    auto& simulator = LoadSimulator((*order)->GetInfo().m_fields.m_security);
    simulator.Update(*order, executionReport);
  }

  template<typename MarketDataClientType, typename TimeClientType>
  void SimulationOrderExecutionDriver<MarketDataClientType, TimeClientType>::
      Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataClient->Open();
      m_timeClient->Open();
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename MarketDataClientType, typename TimeClientType>
  void SimulationOrderExecutionDriver<MarketDataClientType, TimeClientType>::
      Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename MarketDataClientType, typename TimeClientType>
  void SimulationOrderExecutionDriver<MarketDataClientType, TimeClientType>::
      Shutdown() {
    m_securityOrderSimulators.Clear();
    m_openState.SetClosed();
  }

  template<typename MarketDataClientType, typename TimeClientType>
  typename SimulationOrderExecutionDriver<MarketDataClientType,
      TimeClientType>::SecurityOrderSimulator& SimulationOrderExecutionDriver<
      MarketDataClientType, TimeClientType>::LoadSimulator(
      const Security& security) {
    return *m_securityOrderSimulators.GetOrInsert(security,
      [&] {
        return std::make_shared<SecurityOrderSimulator>(*m_marketDataClient,
          security, Beam::Ref(*m_timeClient));
      });
  }
}
}

#endif
