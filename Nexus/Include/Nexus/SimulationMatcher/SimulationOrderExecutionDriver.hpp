#ifndef NEXUS_SIMULATION_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_SIMULATION_ORDER_EXECUTION_DRIVER_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/SimulationMatcher/SecurityOrderSimulator.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * An OrderExecutionDriver that simulates transactions.
   * @param <M> The type of MarketDataClient to use.
   * @param <T> The type of TimeClient used for Order timestamps.
   */
  template<MarketDataService::IsMarketDataClient M, typename T>
  class SimulationOrderExecutionDriver {
    public:

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::GetTryDereferenceType<M>;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /**
       * Constructs a SimulationOrderExecutionDriver.
       * @param market_data_client Initializes the MarketDataClient.
       * @param time_client Initializes the TimeClient.
       */
      template<Beam::Initializes<M> MF, Beam::Initializes<T> TF>
      SimulationOrderExecutionDriver(MF&& market_data_client, TF&& time_client);

      ~SimulationOrderExecutionDriver();
      std::shared_ptr<const Order> recover(
        const SequencedAccountOrderRecord& record);
      void add(const std::shared_ptr<const Order>& order);
      std::shared_ptr<const Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      using SecurityOrderSimulator =
        OrderExecutionService::SecurityOrderSimulator<TimeClient*>;
      Beam::GetOptionalLocalPtr<M> m_market_data_client;
      Beam::GetOptionalLocalPtr<T> m_time_client;
      Beam::SynchronizedUnorderedMap<OrderId, std::shared_ptr<PrimitiveOrder>>
        m_orders;
      OrderId m_next_order_id;
      Beam::SynchronizedUnorderedMap<Security,
        std::unique_ptr<SecurityOrderSimulator>, Beam::Threading::Mutex>
          m_simulators;
      Beam::IO::OpenState m_open_state;

      SimulationOrderExecutionDriver(
        const SimulationOrderExecutionDriver&) = delete;
      SimulationOrderExecutionDriver& operator =(
        const SimulationOrderExecutionDriver&) = delete;
      SecurityOrderSimulator& load(const Security& security);
  };

  template<typename M, typename T>
  SimulationOrderExecutionDriver(M&&, T&&) -> SimulationOrderExecutionDriver<
    std::remove_reference_t<M>, std::remove_reference_t<T>>;

  template<MarketDataService::IsMarketDataClient M, typename T>
  template<Beam::Initializes<M> MF, Beam::Initializes<T> TF>
  SimulationOrderExecutionDriver<M, T>::SimulationOrderExecutionDriver(
    MF&& market_data_client, TF&& time_client)
    : m_market_data_client(std::forward<MF>(market_data_client)),
      m_time_client(std::forward<TF>(time_client)),
      m_next_order_id(1) {}

  template<MarketDataService::IsMarketDataClient M, typename T>
  SimulationOrderExecutionDriver<M, T>::~SimulationOrderExecutionDriver() {
    close();
  }

  template<MarketDataService::IsMarketDataClient M, typename T>
  std::shared_ptr<const Order> SimulationOrderExecutionDriver<M, T>::recover(
      const SequencedAccountOrderRecord& record) {
    auto order = std::make_shared<PrimitiveOrder>(**record);
    m_orders.Insert((*record)->m_info.m_id, order);
    auto& simulator = load((*record)->m_info.m_fields.m_security);
    simulator.recover(order);
    return order;
  }

  template<MarketDataService::IsMarketDataClient M, typename T>
  void SimulationOrderExecutionDriver<M, T>::add(
    const std::shared_ptr<const Order>& order) {}

  template<MarketDataService::IsMarketDataClient M, typename T>
  std::shared_ptr<const Order> SimulationOrderExecutionDriver<M, T>::submit(
      const OrderInfo& info) {
    auto order = std::make_shared<PrimitiveOrder>(info);
    m_orders.Insert(info.m_id, order);
    auto& simulator = load(info.m_fields.m_security);
    simulator.submit(order);
    return order;
  }

  template<MarketDataService::IsMarketDataClient M, typename T>
  void SimulationOrderExecutionDriver<M, T>::cancel(
      const OrderExecutionSession& session, OrderId id) {
    if(auto order = m_orders.Find(id)) {
      auto& simulator = load((*order)->get_info().m_fields.m_security);
      simulator.cancel(*order);
    }
  }

  template<MarketDataService::IsMarketDataClient M, typename T>
  void SimulationOrderExecutionDriver<M, T>::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    if(auto order = m_orders.Find(id)) {
      auto& simulator = load((*order)->get_info().m_fields.m_security);
      simulator.update(*order, report);
    }
  }

  template<MarketDataService::IsMarketDataClient M, typename T>
  void SimulationOrderExecutionDriver<M, T>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_simulators.Clear();
    m_open_state.Close();
  }

  template<MarketDataService::IsMarketDataClient M, typename T>
  typename SimulationOrderExecutionDriver<M, T>::SecurityOrderSimulator&
      SimulationOrderExecutionDriver<M, T>::load(const Security& security) {
    return *m_simulators.GetOrInsert(security, [&] {
      return std::make_unique<SecurityOrderSimulator>(
        *m_market_data_client, security, &*m_time_client);
    });
  }
}

#endif
