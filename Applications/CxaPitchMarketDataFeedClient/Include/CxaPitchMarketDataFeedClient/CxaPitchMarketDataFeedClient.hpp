#ifndef CXA_PITCH_MARKET_DATA_FEED_CLIENT_HPP
#define CXA_PITCH_MARKET_DATA_FEED_CLIENT_HPP
#include <functional>
#include <tuple>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/Expect.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchConfiguration.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchReport.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"

namespace Nexus {

  /**
   * Publishes the market data carried by a CXA PITCH feed.
   * @tparam M The type of client used to publish market data.
   * @tparam C The type of client delivering the PITCH messages.
   */
  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  class CxaPitchMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::dereference_t<M>;

      /** The type of client delivering the PITCH messages. */
      using Client = Beam::dereference_t<C>;

      /**
       * Constructs a CxaPitchMarketDataFeedClient.
       * @param config The configuration to use.
       * @param feed_client Initializes the MarketDataFeedClient.
       * @param client Initializes the client delivering the PITCH messages.
       */
      template<Beam::Initializes<M> MF, Beam::Initializes<C> CF>
      CxaPitchMarketDataFeedClient(
        CxaPitchConfiguration config, MF&& feed_client, CF&& client);

      ~CxaPitchMarketDataFeedClient();

      /** Returns the exception that stopped message reception, if any. */
      std::exception_ptr get_exception() const;

      void close();

    private:
      struct OrderEntry {
        Ticker m_ticker;
        Money m_price;
        Side m_side;
        std::string m_pid;
        std::uint32_t m_quantity;
      };
      CxaPitchConfiguration m_config;
      Beam::local_ptr_t<M> m_feed_client;
      Beam::local_ptr_t<C> m_client;
      std::unordered_map<std::string, OrderEntry> m_orders;
      boost::posix_time::ptime m_timestamp;
      Beam::Sync<std::exception_ptr> m_exception;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      CxaPitchMarketDataFeedClient(
        const CxaPitchMarketDataFeedClient&) = delete;
      CxaPitchMarketDataFeedClient& operator =(
        const CxaPitchMarketDataFeedClient&) = delete;
      void publish(const Ticker& ticker, boost::posix_time::ptime timestamp,
        Money price, std::uint32_t quantity, char code, Side side,
        std::string pid, std::string contra_pid);
      void clear();
      void add(const CxaPitchAddOrder& message);
      template<typename E> requires std::same_as<E, CxaPitchOrderExecuted> ||
        std::same_as<E, CxaPitchOrderExecutedAtPrice>
      void execute(const E& message);
      void reduce(
        typename std::unordered_map<std::string, OrderEntry>::iterator order,
        std::uint32_t quantity);
      void reduce(const CxaPitchReduceSize& message);
      void modify(const CxaPitchModifyOrder& message);
      void remove(const CxaPitchDeleteOrder& message);
      void report(const CxaPitchTrade& message);
      void report(const CxaPitchAuctionUpdate& message);
      void dispatch(const CxaPitchMessage& message);
      void read_loop();
  };

  template<typename M, typename C>
  CxaPitchMarketDataFeedClient(CxaPitchConfiguration, M&&, C&&) ->
    CxaPitchMarketDataFeedClient<
      std::remove_cvref_t<M>, std::remove_cvref_t<C>>;

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  template<Beam::Initializes<M> MF, Beam::Initializes<C> CF>
  CxaPitchMarketDataFeedClient<M, C>::CxaPitchMarketDataFeedClient(
      CxaPitchConfiguration config, MF&& feed_client, CF&& client)
      try : m_config(std::move(config)),
            m_feed_client(std::forward<MF>(feed_client)),
            m_client(std::forward<CF>(client)) {
    m_read_loop = Beam::spawn(std::bind_front(
      &CxaPitchMarketDataFeedClient::read_loop, this));
  } catch(const std::exception&) {
    Beam::throw_nested_with_location(Beam::ConnectException(
      "Unable to initialize the CXA PITCH market data feed client."));
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  CxaPitchMarketDataFeedClient<M, C>::~CxaPitchMarketDataFeedClient() {
    close();
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  std::exception_ptr CxaPitchMarketDataFeedClient<M, C>::get_exception() const {
    return m_exception.load();
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  void CxaPitchMarketDataFeedClient<M, C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_client->close();
    m_feed_client->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  void CxaPitchMarketDataFeedClient<M, C>::publish(const Ticker& ticker,
      boost::posix_time::ptime timestamp, Money price, std::uint32_t quantity,
      char code, Side side, std::string pid, std::string contra_pid) {
    auto condition = TimeAndSale::Condition();
    if(code == ' ') {
      condition.m_type = TimeAndSale::Condition::Type::REGULAR;
      condition.m_code = "@";
    } else {
      condition.m_code = std::string(1, code);
      if(code == 'O' || code == 'H') {
        condition.m_type = TimeAndSale::Condition::Type::OPEN;
      } else if(code == 'C') {
        condition.m_type = TimeAndSale::Condition::Type::CLOSE;
      }
    }
    auto [buyer, seller] = [&] {
      if(side == Side::ASK) {
        return std::tuple(std::move(contra_pid), std::move(pid));
      }
      if(side == Side::BID) {
        return std::tuple(std::move(pid), std::move(contra_pid));
      }
      return std::tuple(std::string(), std::string());
    }();
    m_feed_client->publish(TickerTimeAndSale(
      TimeAndSale(timestamp, price, quantity, std::move(condition),
        m_config.m_mpid, std::move(buyer), std::move(seller)), ticker));
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  void CxaPitchMarketDataFeedClient<M, C>::clear() {
    for(auto& order : m_orders) {
      m_feed_client->remove_order(order.first, m_timestamp);
    }
    m_orders.clear();
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  void CxaPitchMarketDataFeedClient<M, C>::add(
      const CxaPitchAddOrder& message) {
    m_timestamp = message.m_timestamp;
    auto id = std::to_string(message.m_order_id);
    auto ticker = Ticker(message.m_symbol, m_config.m_primary_venue);
    auto order = m_orders.insert_or_assign(
      std::move(id), OrderEntry(ticker, message.m_price, message.m_side,
        message.m_pid, message.m_quantity)).first;
    if(message.m_quantity == 0) {
      return;
    }
    m_feed_client->add_order(ticker, m_config.m_disseminating_venue,
      m_config.m_mpid, false, order->first, message.m_side, message.m_price,
      message.m_quantity, message.m_timestamp);
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  template<typename E> requires std::same_as<E, CxaPitchOrderExecuted> ||
    std::same_as<E, CxaPitchOrderExecutedAtPrice>
  void CxaPitchMarketDataFeedClient<M, C>::execute(const E& message) {
    m_timestamp = message.m_timestamp;
    if(message.m_executed_quantity == 0) {
      return;
    }
    auto id = std::to_string(message.m_order_id);
    m_feed_client->offset_order_size(
      id, -Quantity(message.m_executed_quantity), message.m_timestamp);
    auto order = m_orders.find(id);
    if(order != m_orders.end()) {
      auto [price, code] = [&] {
        if constexpr(std::same_as<E, CxaPitchOrderExecutedAtPrice>) {
          return std::pair(message.m_price, message.m_execution_type);
        } else {
          return std::pair(order->second.m_price, ' ');
        }
      }();
      publish(order->second.m_ticker, message.m_timestamp, price,
        message.m_executed_quantity, code, order->second.m_side,
        order->second.m_pid, message.m_contra_pid);
      reduce(order, message.m_executed_quantity);
    }
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  void CxaPitchMarketDataFeedClient<M, C>::reduce(
      typename std::unordered_map<std::string, OrderEntry>::iterator order,
      std::uint32_t quantity) {
    if(order->second.m_quantity > quantity) {
      order->second.m_quantity -= quantity;
    } else {
      m_orders.erase(order);
    }
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  void CxaPitchMarketDataFeedClient<M, C>::reduce(
      const CxaPitchReduceSize& message) {
    m_timestamp = message.m_timestamp;
    if(message.m_cancelled_quantity == 0) {
      return;
    }
    auto id = std::to_string(message.m_order_id);
    m_feed_client->offset_order_size(
      id, -Quantity(message.m_cancelled_quantity), message.m_timestamp);
    auto order = m_orders.find(id);
    if(order != m_orders.end()) {
      reduce(order, message.m_cancelled_quantity);
    }
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  void CxaPitchMarketDataFeedClient<M, C>::modify(
      const CxaPitchModifyOrder& message) {
    m_timestamp = message.m_timestamp;
    auto id = std::to_string(message.m_order_id);
    auto order = Beam::lookup(m_orders, id);
    if(!order) {
      return;
    }
    if(message.m_quantity == 0) {
      m_feed_client->remove_order(id, message.m_timestamp);
    } else {
      m_feed_client->add_order(order->m_ticker, m_config.m_disseminating_venue,
        m_config.m_mpid, false, id, order->m_side, message.m_price,
        message.m_quantity, message.m_timestamp);
    }
    order->m_price = message.m_price;
    order->m_quantity = message.m_quantity;
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  void CxaPitchMarketDataFeedClient<M, C>::remove(
      const CxaPitchDeleteOrder& message) {
    m_timestamp = message.m_timestamp;
    auto id = std::to_string(message.m_order_id);
    m_feed_client->remove_order(id, message.m_timestamp);
    m_orders.erase(id);
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  void CxaPitchMarketDataFeedClient<M, C>::report(
      const CxaPitchTrade& message) {
    m_timestamp = message.m_timestamp;
    if(message.m_quantity == 0) {
      return;
    }
    auto side = [&] {
      if(auto order =
          Beam::lookup(m_orders, std::to_string(message.m_order_id))) {
        return order->m_side;
      }
      return Side(Side::NONE);
    }();
    auto code = [&] {
      if(message.m_trade_type == 'O' || message.m_trade_type == 'C' ||
          message.m_trade_type == 'H') {
        return message.m_trade_type;
      }
      return message.m_trade_report_type;
    }();
    publish(Ticker(message.m_symbol, m_config.m_primary_venue),
      message.m_timestamp, message.m_price, message.m_quantity, code, side,
      message.m_pid, message.m_contra_pid);
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  void CxaPitchMarketDataFeedClient<M, C>::report(
      const CxaPitchAuctionUpdate& message) {
    m_timestamp = message.m_timestamp;
    auto [side, size] = [&] {
      if(message.m_buy_shares == message.m_sell_shares) {
        return std::tuple(Side(Side::NONE), std::uint32_t(0));
      }
      if(message.m_buy_shares > message.m_sell_shares) {
        return std::tuple(
          Side(Side::BID), message.m_buy_shares - message.m_sell_shares);
      }
      return std::tuple(
        Side(Side::ASK), message.m_sell_shares - message.m_buy_shares);
    }();
    m_feed_client->publish(VenueOrderImbalance(
      OrderImbalance(Ticker(message.m_symbol, m_config.m_primary_venue), side,
        size, message.m_indicative_price, message.m_timestamp),
      m_config.m_disseminating_venue));
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  void CxaPitchMarketDataFeedClient<M, C>::dispatch(
      const CxaPitchMessage& message) {
    visit(message,
      [&] (const CxaPitchAddOrder& message) { add(message); },
      [&] (const CxaPitchOrderExecuted& message) { execute(message); },
      [&] (const CxaPitchOrderExecutedAtPrice& message) { execute(message); },
      [&] (const CxaPitchReduceSize& message) { reduce(message); },
      [&] (const CxaPitchModifyOrder& message) { modify(message); },
      [&] (const CxaPitchDeleteOrder& message) { remove(message); },
      [&] (const CxaPitchTrade& message) { report(message); },
      [&] (const CxaPitchAuctionUpdate& message) { report(message); },
      [&] (const CxaPitchUnitClear&) { clear(); });
  }

  template<typename M, typename C> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsCxaPitchClient<Beam::dereference_t<C>>
  void CxaPitchMarketDataFeedClient<M, C>::read_loop() {
    while(m_open_state.is_open()) {
      auto message = CxaPitchMessage();
      try {
        message = m_client->read();
      } catch(const std::exception&) {
        if(m_open_state.is_open()) {
          m_exception = std::current_exception();
        }
        break;
      }
      if(!m_open_state.is_open()) {
        break;
      }
      try {
        if(m_config.m_is_logging_messages) {
          visit(message, [] (const auto& message) {
            print([&] (auto& out) {
              out << message;
            });
          });
        }
        dispatch(message);
      } catch(const CxaPitchParserException& e) {
        if(!m_open_state.is_open()) {
          break;
        }
        print([&] (auto& out) {
          out << "(bad_message " << e.what() << ')';
        });
      } catch(const std::exception&) {
        if(m_open_state.is_open()) {
          m_exception = std::current_exception();
        }
        break;
      }
    }
  }
}

#endif
