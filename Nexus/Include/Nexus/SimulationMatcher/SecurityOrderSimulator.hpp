#ifndef NEXUS_SECURITY_ORDER_SIMULATOR_HPP
#define NEXUS_SECURITY_ORDER_SIMULATOR_HPP
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {

  /**
   * Handles simulating Orders submitted for a specific Security.
   * @param <T> The type of TimeClient used for Order timestamps.
   */
  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  class SecurityOrderSimulator {
    public:

      /** The type of TimeClient used for Order timestamps. */
      using TimeClient = Beam::dereference_t<T>;

      /**
       * Constructs a SecurityOrderSimulator.
       * @param market_data_client The MarketDataClient to query.
       * @param security The Security to simulate Order executions for.
       * @param time_client The TimeClient used for Order timestamps.
       */
      template<Beam::Initializes<T> TF>
      SecurityOrderSimulator(IsMarketDataClient auto& market_data_client,
        const Security& security, TF&& time_client);

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

    private:
      Beam::local_ptr_t<T> m_time_client;
      boost::gregorian::date m_date;
      boost::posix_time::ptime m_market_close_time;
      bool m_is_moc_pending;
      std::vector<std::shared_ptr<PrimitiveOrder>> m_orders;
      BboQuote m_bbo;
      Beam::RoutineTaskQueue m_tasks;

      SecurityOrderSimulator(const SecurityOrderSimulator&) = delete;
      SecurityOrderSimulator& operator =(
        const SecurityOrderSimulator&) = delete;
      void set_session_timestamps(boost::posix_time::ptime timestamp);
      OrderStatus fill(PrimitiveOrder& order, Money price);
      OrderStatus update(PrimitiveOrder& order);
      void on_bbo(const BboQuote& bbo);
      void on_time_and_sale(const TimeAndSale& time_and_sale);
  };

  template<typename T>
  SecurityOrderSimulator(IsMarketDataClient auto&, const Security&, T&&) ->
    SecurityOrderSimulator<std::remove_cvref_t<T>>;

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  template<Beam::Initializes<T> TF>
  SecurityOrderSimulator<T>::SecurityOrderSimulator(
      IsMarketDataClient auto& market_data_client, const Security& security,
      TF&& time_client)
      : m_time_client(std::forward<TF>(time_client)) {
    set_session_timestamps(m_time_client->get_time());
    auto snapshot = std::make_shared<Beam::Queue<BboQuote>>();
    market_data_client.query(Beam::make_latest_query(security), snapshot);
    try {
      m_bbo = snapshot->pop();
    } catch(const std::exception&) {
      return;
    }
    auto query = Beam::make_current_query(security);
    market_data_client.query(query, m_tasks.get_slot<BboQuote>(
      std::bind_front(&SecurityOrderSimulator::on_bbo, this)));
    market_data_client.query(query, m_tasks.get_slot<TimeAndSale>(
      std::bind_front(&SecurityOrderSimulator::on_time_and_sale, this)));
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void SecurityOrderSimulator<T>::recover(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.push([=, this] {
      m_orders.push_back(order);
      update(*order);
    });
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void SecurityOrderSimulator<T>::submit(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.push([=, this] {
      auto is_live = true;
      order->with([&] (auto status, const auto& reports) {
        auto& last_report = reports.back();
        auto next_status = [&] {
          if(m_bbo.m_bid.m_price == Money::ZERO ||
              m_bbo.m_ask.m_price == Money::ZERO) {
            is_live = false;
            return OrderStatus::REJECTED;
          }
          return OrderStatus::NEW;
        }();
        auto updated_report =
          make_update(last_report, next_status, order->get_info().m_timestamp);
        order->update(updated_report);
      });
      if(is_live) {
        m_orders.push_back(order);
        update(*order);
      }
    });
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void SecurityOrderSimulator<T>::cancel(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.push([=, this] {
      order->with([&] (auto status, const auto& reports) {
        if(is_terminal(status) || reports.empty()) {
          return;
        }
        auto pending_cancel_report = make_update(reports.back(),
          OrderStatus::PENDING_CANCEL, m_time_client->get_time());
        order->update(pending_cancel_report);
        auto cancel_report = make_update(
          reports.back(), OrderStatus::CANCELED, m_time_client->get_time());
        order->update(cancel_report);
      });
    });
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void SecurityOrderSimulator<T>::update(
      const std::shared_ptr<PrimitiveOrder>& order,
      const ExecutionReport& report) {
    m_tasks.push([=, this] {
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
        if(updated_report.m_timestamp.is_special()) {
          updated_report.m_timestamp = m_time_client->get_time();
        }
        order->update(updated_report);
      });
    });
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void SecurityOrderSimulator<T>::set_session_timestamps(
      boost::posix_time::ptime timestamp) {
    m_date = timestamp.date();
    auto eastern_timestamp = Beam::adjust_date_time(
      timestamp, "UTC", "Eastern_Time", get_default_time_zone_database());
    m_market_close_time = Beam::adjust_date_time(
      boost::posix_time::ptime(eastern_timestamp.date(),
        boost::posix_time::hours(16)), "Eastern_Time", "UTC",
      get_default_time_zone_database());
    m_is_moc_pending = timestamp < m_market_close_time;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  OrderStatus SecurityOrderSimulator<T>::fill(
      PrimitiveOrder& order, Money price) {
    order.with([&] (auto status, const auto& reports) {
      auto& last_report = reports.back();
      auto updated_report =
        make_update(last_report, OrderStatus::FILLED, m_time_client->get_time());
      updated_report.m_last_quantity = order.get_info().m_fields.m_quantity;
      updated_report.m_last_price = price;
      order.update(updated_report);
    });
    return OrderStatus::FILLED;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  OrderStatus SecurityOrderSimulator<T>::update(PrimitiveOrder& order) {
    return order.with([&] (auto status, const auto& reports) {
      auto side = order.get_info().m_fields.m_side;
      if(status == OrderStatus::PENDING_NEW || is_terminal(status) ||
          order.get_info().m_fields.m_time_in_force.get_type() ==
            TimeInForce::Type::MOC) {
        return status;
      }
      if(order.get_info().m_fields.m_type == OrderType::MARKET) {
        auto price = pick(side, m_bbo.m_bid.m_price, m_bbo.m_ask.m_price);
        return fill(order, price);
      } else if(side == Side::BID && m_bbo.m_ask.m_price <=
          order.get_info().m_fields.m_price) {
        return fill(order, m_bbo.m_ask.m_price);
      } else if(side == Side::ASK && m_bbo.m_bid.m_price >=
          order.get_info().m_fields.m_price) {
        return fill(order, m_bbo.m_bid.m_price);
      }
      return status;
    });
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void SecurityOrderSimulator<T>::on_bbo(const BboQuote& bbo) {
    if(bbo.m_timestamp.date() != m_date) {
      set_session_timestamps(bbo.m_timestamp);
    }
    m_bbo = bbo;
    std::erase_if(m_orders, [&] (auto& order) {
      return is_terminal(update(*order));
    });
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void SecurityOrderSimulator<T>::on_time_and_sale(
      const TimeAndSale& time_and_sale) {
    if(time_and_sale.m_timestamp.date() != m_date) {
      set_session_timestamps(time_and_sale.m_timestamp);
    }
    if(m_is_moc_pending && time_and_sale.m_timestamp >= m_market_close_time &&
        time_and_sale.m_market_center == "TSE") {
      m_is_moc_pending = false;
      auto closing_price = time_and_sale.m_price;
      std::erase_if(m_orders, [&] (auto& order) {
        if(order->get_info().m_fields.m_time_in_force.get_type() ==
            TimeInForce::Type::MOC) {
          fill(*order, closing_price);
          return true;
        } else {
          return false;
        }
      });
    }
  }
}

#endif
