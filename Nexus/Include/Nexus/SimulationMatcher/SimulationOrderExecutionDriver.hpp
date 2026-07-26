#ifndef NEXUS_SIMULATION_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_SIMULATION_ORDER_EXECUTION_DRIVER_HPP
#include <functional>
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
#include "Nexus/SimulationMatcher/SimulationExecutionReportQueue.hpp"
#include "Nexus/SimulationMatcher/TickerOrderSimulator.hpp"

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

      /**
       * Sets the callback invoked the first time a Ticker is simulated.
       * @param slot The callback to invoke.
       */
      void set_ticker_slot(std::function<void (const Ticker&)> slot);

      /**
       * Updates a Ticker's simulation with a BboQuote.
       * @param ticker The Ticker to update.
       * @param bbo The BboQuote to update the simulation with.
       */
      void update(const Ticker& ticker, const BboQuote& bbo);

      /**
       * Updates a Ticker's simulation with a TimeAndSale.
       * @param ticker The Ticker to update.
       * @param time_and_sale The TimeAndSale to update the simulation with.
       */
      void update(const Ticker& ticker, const TimeAndSale& time_and_sale);

      /**
       * Sets the callback invoked whenever an ExecutionReport is queued, used
       * to wake up whoever flushes the queue.
       * @param slot The callback to invoke.
       */
      void set_execution_report_slot(std::function<void ()> slot);

      /**
       * Publishes the next queued ExecutionReport and then flushes all pending
       * Routines.
       * @return <code>true</code> iff a report was published.
       */
      bool flush_next_execution_report();

      /**
       * Publishes every queued ExecutionReport, flushing all pending Routines
       * after each one.
       */
      void flush_execution_reports();

      std::vector<std::shared_ptr<Order>> restore(
        const Beam::DirectoryEntry& account, const InventorySnapshot& snapshot,
        const std::vector<SequencedOrderRecord>& records);
      void add(const std::shared_ptr<Order>& order);
      std::shared_ptr<Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      using TickerOrderSimulator = Nexus::TickerOrderSimulator<TimeClient*>;
      Beam::local_ptr_t<M> m_market_data_client;
      Beam::local_ptr_t<T> m_time_client;
      std::shared_ptr<SimulationExecutionReportQueue> m_reports;
      std::function<void (const Ticker&)> m_ticker_slot;
      Beam::SynchronizedUnorderedMap<OrderId, std::shared_ptr<PrimitiveOrder>>
        m_orders;
      OrderId m_next_order_id;
      Beam::SynchronizedUnorderedMap<Ticker,
        std::unique_ptr<TickerOrderSimulator>, Beam::Mutex> m_simulators;
      Beam::OpenState m_open_state;

      SimulationOrderExecutionDriver(
        const SimulationOrderExecutionDriver&) = delete;
      SimulationOrderExecutionDriver& operator =(
        const SimulationOrderExecutionDriver&) = delete;
      TickerOrderSimulator* find(const Ticker& ticker);
      TickerOrderSimulator& load(const Ticker& ticker);
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
      m_reports(std::make_shared<SimulationExecutionReportQueue>()),
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
  void SimulationOrderExecutionDriver<M, T>::set_ticker_slot(
      std::function<void (const Ticker&)> slot) {
    m_ticker_slot = std::move(slot);
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void SimulationOrderExecutionDriver<M, T>::update(
      const Ticker& ticker, const BboQuote& bbo) {
    if(auto simulator = find(ticker)) {
      simulator->update(bbo);
    }
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void SimulationOrderExecutionDriver<M, T>::update(
      const Ticker& ticker, const TimeAndSale& time_and_sale) {
    if(auto simulator = find(ticker)) {
      simulator->update(time_and_sale);
    }
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void SimulationOrderExecutionDriver<M, T>::set_execution_report_slot(
      std::function<void()> slot) {
    m_reports->set_slot(std::move(slot));
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  bool SimulationOrderExecutionDriver<M, T>::flush_next_execution_report() {
    return m_reports->flush_next();
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void SimulationOrderExecutionDriver<M, T>::flush_execution_reports() {
    m_reports->flush();
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  std::vector<std::shared_ptr<Order>>
      SimulationOrderExecutionDriver<M, T>::restore(
        const Beam::DirectoryEntry& account, const InventorySnapshot& snapshot,
        const std::vector<SequencedOrderRecord>& records) {
    auto orders = std::vector<std::shared_ptr<Order>>();
    for(auto& record : records) {
      auto order = std::make_shared<PrimitiveOrder>(*record);
      m_orders.insert(record->m_info.m_id, order);
      auto& simulator = load(record->m_info.m_fields.m_ticker);
      simulator.recover(order);
      orders.push_back(order);
    }
    return orders;
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
    auto& simulator = load(info.m_fields.m_ticker);
    simulator.submit(order);
    return order;
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  void SimulationOrderExecutionDriver<M, T>::cancel(
      const OrderExecutionSession& session, OrderId id) {
    if(auto order = m_orders.find(id)) {
      auto& simulator = load((*order)->get_info().m_fields.m_ticker);
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
      auto& simulator = load((*order)->get_info().m_fields.m_ticker);
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
  typename SimulationOrderExecutionDriver<M, T>::TickerOrderSimulator*
      SimulationOrderExecutionDriver<M, T>::find(const Ticker& ticker) {
    return m_simulators.with([&] (auto& simulators) -> TickerOrderSimulator* {
      auto i = simulators.find(ticker);
      if(i == simulators.end()) {
        return nullptr;
      }
      return i->second.get();
    });
  }

  template<typename M, typename T> requires
    IsMarketDataClient<Beam::dereference_t<M>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>>
  typename SimulationOrderExecutionDriver<M, T>::TickerOrderSimulator&
      SimulationOrderExecutionDriver<M, T>::load(const Ticker& ticker) {
    auto [simulator, is_new] = m_simulators.with(
      [&] (auto& simulators) -> std::pair<TickerOrderSimulator*, bool> {
        auto i = simulators.find(ticker);
        if(i != simulators.end()) {
          return {i->second.get(), false};
        }
        i = simulators.emplace(ticker, std::make_unique<TickerOrderSimulator>(
          *m_market_data_client, ticker, &*m_time_client, m_reports)).first;
        return {i->second.get(), true};
      });
    if(is_new && m_ticker_slot) {
      m_ticker_slot(ticker);
    }
    return *simulator;
  }
}

#endif
