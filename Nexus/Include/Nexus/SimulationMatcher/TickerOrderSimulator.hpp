#ifndef NEXUS_TICKER_ORDER_SIMULATOR_HPP
#define NEXUS_TICKER_ORDER_SIMULATOR_HPP
#include <memory>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/FixTags.hpp"
#include "Nexus/Definitions/StandardTimeZones.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/SimulationMatcher/SimulationExecutionReportQueue.hpp"

namespace Nexus {

  /**
   * Handles simulating Orders submitted for a specific Ticker.
   * @param <T> The type of TimeClient used for Order timestamps.
   */
  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  class TickerOrderSimulator {
    public:

      /** The type of TimeClient used for Order timestamps. */
      using TimeClient = Beam::dereference_t<T>;

      /**
       * Constructs a TickerOrderSimulator.
       * @param market_data_client The MarketDataClient to query.
       * @param ticker The Ticker to simulate Order executions for.
       * @param time_client The TimeClient used for Order timestamps.
       * @param reports The queue to publish ExecutionReports through.
       */
      template<Beam::Initializes<T> TF>
      TickerOrderSimulator(IsMarketDataClient auto& market_data_client,
        const Ticker& ticker, TF&& time_client,
        std::shared_ptr<SimulationExecutionReportQueue> reports);

      /**
       * Recovers a previously submitted Order.
       * @param order The Order to recover.
       */
      void recover(const std::shared_ptr<PrimitiveOrder>& order);

      /**
       * Submits an Order for simulated Order entry.
       * @param order The Order to submit.
       */
      void submit(const std::shared_ptr<PrimitiveOrder>& order);

      /**
       * Cancels a previously submitted order.
       * @param order The Order to cancel.
       */
      void cancel(const std::shared_ptr<PrimitiveOrder>& order);

      /**
       * Updates an Order.
       * @param order The Order to update.
       * @param report The ExecutionReport containing the update.
       */
      void update(const std::shared_ptr<PrimitiveOrder>& order,
        const ExecutionReport& report);

      /**
       * Updates the simulation with a BboQuote.
       * @param bbo The BboQuote to update the simulation with.
       */
      void update(const BboQuote& bbo);

      /**
       * Updates the simulation with a TimeAndSale.
       * @param time_and_sale The TimeAndSale to update the simulation with.
       */
      void update(const TimeAndSale& time_and_sale);

    private:
      struct PeggedOrderEntry {
        std::string m_exec_inst;
        Money m_peg_difference;
        Money m_effective_price;
      };
      Beam::local_ptr_t<T> m_time_client;
      std::shared_ptr<SimulationExecutionReportQueue> m_reports;
      boost::gregorian::date m_date;
      boost::posix_time::ptime m_venue_close_time;
      bool m_is_moc_pending;
      std::vector<std::shared_ptr<PrimitiveOrder>> m_orders;
      std::unordered_map<OrderId, OrderStatus> m_statuses;
      std::unordered_map<OrderId, PeggedOrderEntry> m_pegged_entries;
      BboQuote m_bbo;
      Beam::Mutex m_mutex;

      TickerOrderSimulator(const TickerOrderSimulator&) = delete;
      TickerOrderSimulator& operator =(const TickerOrderSimulator&) = delete;
      void set_session_timestamps(boost::posix_time::ptime timestamp);
      void submit_pegged(const PrimitiveOrder& order);
      void enqueue(const std::shared_ptr<PrimitiveOrder>& order,
        OrderStatus status, boost::posix_time::ptime timestamp,
        Quantity last_quantity, Money last_price);
      OrderStatus fill(
        const std::shared_ptr<PrimitiveOrder>& order, Money price);
      OrderStatus evaluate(
        const std::shared_ptr<PrimitiveOrder>& order, OrderStatus status);
      OrderStatus update_pegged(
        const std::shared_ptr<PrimitiveOrder>& order, OrderStatus status);
      void erase(const std::shared_ptr<PrimitiveOrder>& order);
  };

  template<typename T>
  TickerOrderSimulator(IsMarketDataClient auto&, const Ticker&, T&&,
    std::shared_ptr<SimulationExecutionReportQueue>) ->
      TickerOrderSimulator<std::remove_cvref_t<T>>;

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  template<Beam::Initializes<T> TF>
  TickerOrderSimulator<T>::TickerOrderSimulator(
      IsMarketDataClient auto& market_data_client, const Ticker& ticker,
      TF&& time_client, std::shared_ptr<SimulationExecutionReportQueue> reports)
      : m_time_client(std::forward<TF>(time_client)),
        m_reports(std::move(reports)) {
    set_session_timestamps(m_time_client->get_time());
    auto snapshot = std::make_shared<Beam::Queue<BboQuote>>();
    market_data_client.query(Beam::make_latest_query(ticker), snapshot);
    try {
      m_bbo = snapshot->pop();
    } catch(const std::exception&) {}
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::recover(
      const std::shared_ptr<PrimitiveOrder>& order) {
    auto lock = std::lock_guard(m_mutex);
    auto status = order->with([] (auto status, const auto&) {
      return status;
    });
    if(is_terminal(status)) {
      return;
    }
    if(order->get_info().m_fields.m_type == OrderType::PEGGED) {
      submit_pegged(*order);
    }
    auto next_status = evaluate(order, status);
    if(is_terminal(next_status)) {
      m_pegged_entries.erase(order->get_info().m_id);
      return;
    }
    m_orders.push_back(order);
    m_statuses[order->get_info().m_id] = next_status;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::submit(
      const std::shared_ptr<PrimitiveOrder>& order) {
    auto lock = std::lock_guard(m_mutex);
    if(m_bbo.m_bid.m_price == Money::ZERO ||
        m_bbo.m_ask.m_price == Money::ZERO) {
      enqueue(order, OrderStatus::REJECTED, order->get_info().m_timestamp, 0,
        Money::ZERO);
      return;
    }
    enqueue(
      order, OrderStatus::NEW, order->get_info().m_timestamp, 0, Money::ZERO);
    if(order->get_info().m_fields.m_type == OrderType::PEGGED) {
      submit_pegged(*order);
    }
    auto next_status = evaluate(order, OrderStatus::NEW);
    if(is_terminal(next_status)) {
      m_pegged_entries.erase(order->get_info().m_id);
      return;
    }
    m_orders.push_back(order);
    m_statuses[order->get_info().m_id] = next_status;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::cancel(
      const std::shared_ptr<PrimitiveOrder>& order) {
    auto lock = std::lock_guard(m_mutex);
    auto i = m_statuses.find(order->get_info().m_id);
    if(i == m_statuses.end() || is_terminal(i->second)) {
      return;
    }
    auto timestamp = m_time_client->get_time();
    enqueue(order, OrderStatus::PENDING_CANCEL, timestamp, 0, Money::ZERO);
    enqueue(order, OrderStatus::CANCELED, timestamp, 0, Money::ZERO);
    i->second = OrderStatus::CANCELED;
    erase(order);
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::update(
      const std::shared_ptr<PrimitiveOrder>& order,
      const ExecutionReport& report) {
    auto lock = std::lock_guard(m_mutex);
    auto i = m_statuses.find(order->get_info().m_id);
    if(i != m_statuses.end()) {
      if(is_terminal(i->second)) {
        return;
      }
      i->second = report.m_status;
      if(is_terminal(report.m_status)) {
        erase(order);
      }
    }
    auto timestamp = [&] {
      if(report.m_timestamp.is_special()) {
        return m_time_client->get_time();
      }
      return report.m_timestamp;
    }();
    m_reports->push([=, this] {
      order->with([&] (auto status, const auto& reports) {
        if(is_terminal(status) ||
            reports.empty() && report.m_status != OrderStatus::PENDING_NEW) {
          return;
        }
        auto updated_report = report;
        if(reports.empty()) {
          updated_report.m_sequence = 0;
        } else {
          updated_report.m_sequence = reports.back().m_sequence + 1;
        }
        updated_report.m_timestamp = timestamp;
        order->update(updated_report);
      });
    });
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::update(const BboQuote& bbo) {
    {
      auto lock = std::lock_guard(m_mutex);
      if(bbo.m_timestamp.date() != m_date) {
        set_session_timestamps(bbo.m_timestamp);
      }
      m_bbo = bbo;
    }
    auto index = std::size_t(0);
    while(true) {
      auto has_update = false;
      {
        auto lock = std::lock_guard(m_mutex);
        if(index >= m_orders.size()) {
          break;
        }
        auto order = m_orders[index];
        auto& status = m_statuses[order->get_info().m_id];
        auto next_status = evaluate(order, status);
        if(is_terminal(next_status)) {
          status = next_status;
          erase(order);
          has_update = true;
        }
      }
      if(has_update) {
        m_reports->flush();
      } else {
        ++index;
      }
    }
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::update(const TimeAndSale& time_and_sale) {
    auto closing_price = Money();
    auto is_triggered = false;
    {
      auto lock = std::lock_guard(m_mutex);
      if(time_and_sale.m_timestamp.date() != m_date) {
        set_session_timestamps(time_and_sale.m_timestamp);
      }
      if(m_is_moc_pending && time_and_sale.m_timestamp >= m_venue_close_time &&
          time_and_sale.m_market_center == "TSE") {
        m_is_moc_pending = false;
        is_triggered = true;
        closing_price = time_and_sale.m_price;
      }
    }
    if(!is_triggered) {
      return;
    }
    auto index = std::size_t(0);
    while(true) {
      auto has_update = false;
      auto is_moc = false;
      {
        auto lock = std::lock_guard(m_mutex);
        if(index >= m_orders.size()) {
          break;
        }
        auto order = m_orders[index];
        is_moc = order->get_info().m_fields.m_time_in_force.get_type() ==
          TimeInForce::Type::MOC;
        if(is_moc) {
          auto& status = m_statuses[order->get_info().m_id];
          if(!is_terminal(status)) {
            status = fill(order, closing_price);
            has_update = true;
          }
          erase(order);
        }
      }
      if(has_update) {
        m_reports->flush();
      }
      if(!is_moc) {
        ++index;
      }
    }
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::set_session_timestamps(
      boost::posix_time::ptime timestamp) {
    m_date = timestamp.date();
    auto eastern_timestamp =
      Beam::to_timezone(timestamp, "Etc/UTC", "America/New_York", TIME_ZONES);
    m_venue_close_time = Beam::to_timezone(boost::posix_time::ptime(
      eastern_timestamp.date(), boost::posix_time::hours(16)),
      "America/New_York", "Etc/UTC", TIME_ZONES);
    m_is_moc_pending = timestamp < m_venue_close_time;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::submit_pegged(const PrimitiveOrder& order) {
    auto& fields = order.get_info().m_fields;
    auto entry = PeggedOrderEntry();
    entry.m_exec_inst = PRIMARY_PEG;
    if(auto tag = find_field(fields, EXEC_INST_KEY)) {
      if(auto* value = boost::get<std::string>(&tag->get_value())) {
        auto stream = std::istringstream(*value);
        auto token = std::string();
        while(stream >> token) {
          if(token == PRIMARY_PEG || token == MARKET_PEG ||
              token == MID_PRICE_PEG) {
            entry.m_exec_inst = token;
            break;
          }
        }
      }
    }
    if(auto tag = find_field(fields, PEG_DIFFERENCE_KEY)) {
      if(auto* money = boost::get<Money>(&tag->get_value())) {
        entry.m_peg_difference = *money;
      }
    }
    auto direction = get_direction(fields.m_side);
    auto [same_price, opposite_price] = pick(fields.m_side,
      std::pair(m_bbo.m_ask.m_price, m_bbo.m_bid.m_price),
      std::pair(m_bbo.m_bid.m_price, m_bbo.m_ask.m_price));
    entry.m_effective_price = [&] {
      if(entry.m_exec_inst == MARKET_PEG) {
        return opposite_price;
      } else if(entry.m_exec_inst == MID_PRICE_PEG) {
        return (same_price + opposite_price) / 2;
      }
      return same_price;
    }();
    entry.m_effective_price -= direction * entry.m_peg_difference;
    if(fields.m_price != Money::ZERO &&
        direction * entry.m_effective_price > direction * fields.m_price) {
      entry.m_effective_price = fields.m_price;
    }
    m_pegged_entries[order.get_info().m_id] = entry;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::enqueue(
      const std::shared_ptr<PrimitiveOrder>& order, OrderStatus status,
      boost::posix_time::ptime timestamp, Quantity last_quantity,
      Money last_price) {
    m_reports->push([=] {
      order->with([&] (auto current_status, const auto& reports) {
        if(reports.empty() || is_terminal(current_status)) {
          return;
        }
        auto updated_report = make_update(reports.back(), status, timestamp);
        updated_report.m_last_quantity = last_quantity;
        updated_report.m_last_price = last_price;
        order->update(updated_report);
      });
    });
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  OrderStatus TickerOrderSimulator<T>::fill(
      const std::shared_ptr<PrimitiveOrder>& order, Money price) {
    enqueue(order, OrderStatus::FILLED, m_time_client->get_time(),
      order->get_info().m_fields.m_quantity, price);
    return OrderStatus::FILLED;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  OrderStatus TickerOrderSimulator<T>::evaluate(
      const std::shared_ptr<PrimitiveOrder>& order, OrderStatus status) {
    auto& fields = order->get_info().m_fields;
    if(status == OrderStatus::PENDING_NEW || is_terminal(status) ||
        fields.m_time_in_force.get_type() == TimeInForce::Type::MOC) {
      return status;
    }
    if(fields.m_type == OrderType::MARKET) {
      auto price =
        pick(fields.m_side, m_bbo.m_bid.m_price, m_bbo.m_ask.m_price);
      return fill(order, price);
    } else if(fields.m_type == OrderType::PEGGED) {
      return update_pegged(order, status);
    } else if(fields.m_side == Side::BID &&
        m_bbo.m_ask.m_price <= fields.m_price) {
      return fill(order, m_bbo.m_ask.m_price);
    } else if(fields.m_side == Side::ASK &&
        m_bbo.m_bid.m_price >= fields.m_price) {
      return fill(order, m_bbo.m_bid.m_price);
    }
    return status;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  OrderStatus TickerOrderSimulator<T>::update_pegged(
      const std::shared_ptr<PrimitiveOrder>& order, OrderStatus status) {
    auto i = m_pegged_entries.find(order->get_info().m_id);
    if(i == m_pegged_entries.end()) {
      return status;
    }
    auto& entry = i->second;
    auto side = order->get_info().m_fields.m_side;
    auto direction = get_direction(side);
    auto [same_price, opposite_price] =
      pick(side, std::pair(m_bbo.m_ask.m_price, m_bbo.m_bid.m_price),
        std::pair(m_bbo.m_bid.m_price, m_bbo.m_ask.m_price));
    auto candidate = [&] {
      if(entry.m_exec_inst == MARKET_PEG) {
        return opposite_price;
      } else if(entry.m_exec_inst == MID_PRICE_PEG) {
        return (same_price + opposite_price) / 2;
      }
      return same_price;
    }();
    candidate -= direction * entry.m_peg_difference;
    if(direction * candidate > direction * entry.m_effective_price) {
      entry.m_effective_price = candidate;
    }
    auto limit_price = order->get_info().m_fields.m_price;
    if(limit_price != Money::ZERO &&
        direction * entry.m_effective_price > direction * limit_price) {
      entry.m_effective_price = limit_price;
    }
    if(direction * opposite_price <= direction * entry.m_effective_price) {
      m_pegged_entries.erase(i);
      return fill(order, opposite_price);
    }
    return status;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void TickerOrderSimulator<T>::erase(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_pegged_entries.erase(order->get_info().m_id);
    m_statuses.erase(order->get_info().m_id);
    std::erase(m_orders, order);
  }
}

#endif
