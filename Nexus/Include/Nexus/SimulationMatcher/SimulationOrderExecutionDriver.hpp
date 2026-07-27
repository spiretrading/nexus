#ifndef NEXUS_SIMULATION_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_SIMULATION_ORDER_EXECUTION_DRIVER_HPP
#include <memory>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/SimulationMatcher/PassiveSimulationOrderExecutionDriver.hpp"

namespace Nexus {

  /** An OrderExecutionDriver that simulates transactions. */
  class SimulationOrderExecutionDriver {
    public:

      /**
       * Constructs a SimulationOrderExecutionDriver.
       * @param market_data_client The MarketDataClient to source quotes from.
       * @param time_client The TimeClient used for Order timestamps.
       */
      SimulationOrderExecutionDriver(
        MarketDataClient market_data_client, Beam::TimeClient time_client);

      ~SimulationOrderExecutionDriver();

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
      MarketDataClient m_market_data_client;
      PassiveSimulationOrderExecutionDriver<MarketDataClient, Beam::TimeClient>
        m_driver;
      Beam::RoutineHandlerGroup m_query_routines;
      Beam::RoutineTaskQueue m_tasks;
      Beam::OpenState m_open_state;

      SimulationOrderExecutionDriver(
        const SimulationOrderExecutionDriver&) = delete;
      SimulationOrderExecutionDriver& operator =(
        const SimulationOrderExecutionDriver&) = delete;
      void subscribe(const Ticker& ticker);
  };

  inline SimulationOrderExecutionDriver::SimulationOrderExecutionDriver(
      MarketDataClient market_data_client, Beam::TimeClient time_client)
      : m_market_data_client(market_data_client),
        m_driver(std::move(market_data_client), std::move(time_client)) {
    m_driver.set_ticker_slot([this] (const auto& ticker) {
      subscribe(ticker);
    });
  }

  inline SimulationOrderExecutionDriver::~SimulationOrderExecutionDriver() {
    close();
  }

  inline std::vector<std::shared_ptr<Order>>
      SimulationOrderExecutionDriver::restore(
        const Beam::DirectoryEntry& account, const InventorySnapshot& snapshot,
        const std::vector<SequencedOrderRecord>& records) {
    auto orders = m_driver.restore(account, snapshot, records);
    m_tasks.push([this] {
      m_driver.flush_execution_reports();
    });
    return orders;
  }

  inline void SimulationOrderExecutionDriver::add(
      const std::shared_ptr<Order>& order) {
    m_driver.add(order);
  }

  inline std::shared_ptr<Order> SimulationOrderExecutionDriver::submit(
      const OrderInfo& info) {
    auto order = m_driver.submit(info);
    m_tasks.push([this] {
      m_driver.flush_execution_reports();
    });
    return order;
  }

  inline void SimulationOrderExecutionDriver::cancel(
      const OrderExecutionSession& session, OrderId id) {
    m_driver.cancel(session, id);
    m_tasks.push([this] {
      m_driver.flush_execution_reports();
    });
  }

  inline void SimulationOrderExecutionDriver::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    m_driver.update(session, id, report);
    m_tasks.push([this] {
      m_driver.flush_execution_reports();
    });
  }

  inline void SimulationOrderExecutionDriver::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_tasks.close();
    m_query_routines.wait();
    m_tasks.wait();
    m_driver.close();
    m_open_state.close();
  }

  inline void SimulationOrderExecutionDriver::subscribe(const Ticker& ticker) {
    m_query_routines.add(query_real_time_with_snapshot(m_market_data_client,
      ticker, m_tasks.get_slot<BboQuote>([this, ticker] (const auto& bbo) {
        m_driver.update(ticker, bbo);
      })));
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(Beam::Range::REAL_TIME);
    query.set_interruption_policy(Beam::InterruptionPolicy::RECOVER_DATA);
    m_market_data_client.query(query, m_tasks.get_slot<TimeAndSale>(
      [this, ticker] (const auto& time_and_sale) {
        m_driver.update(ticker, time_and_sale);
      }));
  }
}

#endif
