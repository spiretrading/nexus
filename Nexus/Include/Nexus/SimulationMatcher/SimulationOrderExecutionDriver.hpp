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

namespace Nexus {

  /**
   * An OrderExecutionDriver that simulates transactions.
   * @param <M> The type of MarketDataClient to use.
   * @param <T> The type of TimeClient used for Order timestamps.
   */
  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  class SimulationOrderExecutionDriver {
    public:

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::dereference_t<M>;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::dereference_t<T>;

      /**
       * Constructs a SimulationOrderExecutionDriver.
       * @param market_data_client Initializes the MarketDataClient.
       * @param time_client Initializes the TimeClient.
       */
      template<Beam::Initializes<M> MF, Beam::Initializes<T> TF>
      SimulationOrderExecutionDriver(MF&& market_data_client, TF&& time_client);

      ~SimulationOrderExecutionDriver();

      std::shared_ptr<Order> recover(const SequencedAccountOrderRecord& record);
      void add(const std::shared_ptr<Order>& order);
      std::shared_ptr<Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      using SecurityOrderSimulator = Nexus::SecurityOrderSimulator<TimeClient*>;
      Beam::local_ptr_t<M> m_market_data_client;
      Beam::local_ptr_t<T> m_time_client;
      Beam::SynchronizedUnorderedMap<OrderId, std::shared_ptr<PrimitiveOrder>>
        m_orders;
      OrderId m_next_order_id;
      Beam::SynchronizedUnorderedMap<Security,
        std::unique_ptr<SecurityOrderSimulator>, Beam::Mutex> m_simulators;
      Beam::OpenState m_open_state;

      SimulationOrderExecutionDriver(
        const SimulationOrderExecutionDriver&) = delete;
      SimulationOrderExecutionDriver& operator =(
        const SimulationOrderExecutionDriver&) = delete;
      SecurityOrderSimulator& load(const Security& security);
  };

  template<typename M, typename T>
  SimulationOrderExecutionDriver(M&&, T&&) -> SimulationOrderExecutionDriver<
    std::remove_cvref_t<M>, std::remove_cvref_t<T>>;

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  template<Beam::Initializes<M> MF, Beam::Initializes<T> TF>
  SimulationOrderExecutionDriver<M, T>::SimulationOrderExecutionDriver(
    MF&& market_data_client, TF&& time_client)
    : m_market_data_client(std::forward<MF>(market_data_client)),
      m_time_client(std::forward<TF>(time_client)),
      m_next_order_id(1) {}

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  SimulationOrderExecutionDriver<M, T>::~SimulationOrderExecutionDriver() {
    close();
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  std::shared_ptr<Order> SimulationOrderExecutionDriver<M, T>::recover(
      const SequencedAccountOrderRecord& record) {
    auto order = std::make_shared<PrimitiveOrder>(**record);
    m_orders.insert((*record)->m_info.m_id, order);
    auto& simulator = load((*record)->m_info.m_fields.m_security);
    simulator.recover(order);
    return order;
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void SimulationOrderExecutionDriver<M, T>::add(
    const std::shared_ptr<Order>& order) {}

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  std::shared_ptr<Order> SimulationOrderExecutionDriver<M, T>::submit(
      const OrderInfo& info) {
    auto order = std::make_shared<PrimitiveOrder>(info);
    m_orders.insert(info.m_id, order);
    auto& simulator = load(info.m_fields.m_security);
    simulator.submit(order);
    return order;
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void SimulationOrderExecutionDriver<M, T>::cancel(
      const OrderExecutionSession& session, OrderId id) {
    if(auto order = m_orders.find(id)) {
      auto& simulator = load((*order)->get_info().m_fields.m_security);
      simulator.cancel(*order);
    }
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void SimulationOrderExecutionDriver<M, T>::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    if(auto order = m_orders.find(id)) {
      auto& simulator = load((*order)->get_info().m_fields.m_security);
      simulator.update(*order, report);
    }
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void SimulationOrderExecutionDriver<M, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_simulators.clear();
    m_open_state.close();
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  typename SimulationOrderExecutionDriver<M, T>::SecurityOrderSimulator&
      SimulationOrderExecutionDriver<M, T>::load(const Security& security) {
    return *m_simulators.get_or_insert(security, [&] {
      return std::make_unique<SecurityOrderSimulator>(
        *m_market_data_client, security, &*m_time_client);
    });
  }
}

#endif
