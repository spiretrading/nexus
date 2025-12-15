#ifndef NEXUS_CHIA_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_CHIA_MARKET_DATA_FEED_CLIENT_HPP
#include <string>
#include <unordered_map>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include "ChiaMarketDataFeedClient/ChiaConfiguration.hpp"
#include "ChiaMarketDataFeedClient/PitchMessage.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"

namespace Nexus {

  /**
   * Parses PITCH messages from the CHIA market data feed.
   * @param <M> The type of MarketDataFeedClient used to update the
   *        MarketDataServer.
   * @param <P> The type of client receiving PITCH messages.
   */
  template<typename M, typename P>
  class ChiaMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::dereference_t<M>;

      /** The type of client receiving PITCH messages. */
      using ProtocolClient = Beam::dereference_t<P>;

      /**
       * Constructs a ChiaMarketDataFeedClient.
       * @param config The configuration to use.
       * @param feed_client Initializes the MarketDataFeedClient.
       * @param itch_client The client receiving PITCH messages.
       */
      template<typename MF, typename PF>
      ChiaMarketDataFeedClient(
        ChiaConfiguration config, MF&& feed_client, PF&& itch_client);

      ~ChiaMarketDataFeedClient();

      void close();

    private:
      struct OrderEntry {
        Security m_security;
        Money m_price;
        Side m_side;
        std::string m_mpid;
      };
      ChiaConfiguration m_config;
      Beam::local_ptr_t<M> m_feed_client;
      Beam::local_ptr_t<P> m_protocol_client;
      std::unordered_map<std::string, OrderEntry> m_order_entries;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      static std::string parse_mpid(Beam::Out<const char*> cursor);
      ChiaMarketDataFeedClient(const ChiaMarketDataFeedClient&) = delete;
      ChiaMarketDataFeedClient& operator =(
        const ChiaMarketDataFeedClient&) = delete;
      void handle_add_order_message(const PitchMessage& message);
      void handle_order_executed_message(const PitchMessage& message);
      void handle_reduce_size_message(const PitchMessage& message);
      void handle_modify_order_message(const PitchMessage& message);
      void handle_delete_order_message(const PitchMessage& message);
      void handle_trade_message(const PitchMessage& message);
      void dispatch(const PitchMessage& message);
      void read_loop();
  };

  template<typename M, typename P>
  template<typename MF, typename PF>
  ChiaMarketDataFeedClient<M, P>::ChiaMarketDataFeedClient(
      ChiaConfiguration config, MF&& feed_client, PF&& protocol_client)
      try : m_config(std::move(config)),
            m_feed_client(std::forward<MF>(feed_client)),
            m_protocol_client(std::forward<PF>(protocol_client)),
            m_read_loop(Beam::spawn(
              std::bind_front(&ChiaMarketDataFeedClient::read_loop, this))) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Unable to initialize the CHIA market data feed client."));
  }

  template<typename M, typename P>
  ChiaMarketDataFeedClient<M, P>::~ChiaMarketDataFeedClient() {
    close();
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_protocol_client->close();
    m_feed_client->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename M, typename P>
  std::string ChiaMarketDataFeedClient<M, P>::parse_mpid(
      Beam::Out<const char*> cursor) {
    auto mpid = PitchMessage::parse_alphanumeric(4, Beam::out(cursor));
    if(mpid.empty()) {
      return "AU000";
    }
    return mpid;
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::handle_add_order_message(
      const PitchMessage& message) {
    auto cursor = message.m_payload;
    auto timestamp = PitchMessage::parse_timestamp(Beam::out(cursor));
    auto order_id =
      std::to_string(PitchMessage::parse_uint64(Beam::out(cursor)));
    auto side = PitchMessage::parse_side(Beam::out(cursor));
    auto quantity = PitchMessage::parse_uint32(Beam::out(cursor));
    if(quantity == 0) {
      return;
    }
    auto symbol = PitchMessage::parse_alphanumeric(6, Beam::out(cursor));
    auto price = PitchMessage::parse_price(Beam::out(cursor));
    auto mpid = parse_mpid(Beam::out(cursor));
    auto security = Security(symbol, m_config.m_primary_venue);
    if(m_config.m_is_time_and_sale_feed) {
      m_order_entries[order_id] = OrderEntry(security, price, side, mpid);
    }
    m_feed_client->add_order(security, m_config.m_disseminating_venue,
      m_config.m_mpid, false, order_id, side, price, quantity, timestamp);
    if(m_config.m_is_logging_messages) {
      std::cout << timestamp << ',' << message.m_type << ',' << order_id <<
        ',' << side << ',' << quantity << ',' << symbol << ',' << price <<
        ',' << mpid << std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::handle_order_executed_message(
      const PitchMessage& message) {
    auto cursor = message.m_payload;
    auto timestamp = PitchMessage::parse_timestamp(Beam::out(cursor));
    auto order_id =
      std::to_string(PitchMessage::parse_uint64(Beam::out(cursor)));
    auto executed_quantity = PitchMessage::parse_uint32(Beam::out(cursor));
    if(executed_quantity == 0) {
      return;
    }
    auto execution_id = PitchMessage::parse_uint64(Beam::out(cursor));
    auto contra_order_id = PitchMessage::parse_uint64(Beam::out(cursor));
    auto contra_mpid = parse_mpid(Beam::out(cursor));
    m_feed_client->offset_order_size(
      order_id, -static_cast<std::int32_t>(executed_quantity), timestamp);
    if(m_config.m_is_time_and_sale_feed) {
      if(auto order_entry = Beam::lookup(m_order_entries, order_id)) {
        auto condition = TimeAndSale::Condition();
        condition.m_code = "@";
        auto [buyer_mpid, seller_mpid] = [&] {
          if(order_entry->m_side == Side::BID) {
            return std::tuple(&order_entry->m_mpid, &contra_mpid);
          }
          return std::tuple(&contra_mpid, &order_entry->m_mpid);
        }();
        auto time_and_sale = TimeAndSale(timestamp, order_entry->m_price,
          executed_quantity, std::move(condition), m_config.m_mpid, *buyer_mpid,
          *seller_mpid);
        m_feed_client->publish(SecurityTimeAndSale(
          std::move(time_and_sale), order_entry->m_security));
      }
    }
    if(m_config.m_is_logging_messages) {
      std::cout << timestamp << ',' << message.m_type << ',' << order_id <<
        ',' << executed_quantity << std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::handle_reduce_size_message(
      const PitchMessage& message) {
    auto cursor = message.m_payload;
    auto timestamp = PitchMessage::parse_timestamp(Beam::out(cursor));
    auto order_id =
      std::to_string(PitchMessage::parse_uint64(Beam::out(cursor)));
    auto cancelled_quantity = PitchMessage::parse_uint32(Beam::out(cursor));
    if(cancelled_quantity == 0) {
      return;
    }
    m_feed_client->offset_order_size(
      order_id, -static_cast<std::int32_t>(cancelled_quantity), timestamp);
    if(m_config.m_is_logging_messages) {
      std::cout << timestamp << ',' << message.m_type << ',' << order_id <<
        ',' << cancelled_quantity << std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::handle_modify_order_message(
      const PitchMessage& message) {
    auto cursor = message.m_payload;
    auto timestamp = PitchMessage::parse_timestamp(Beam::out(cursor));
    auto order_id =
      std::to_string(PitchMessage::parse_uint64(Beam::out(cursor)));
    auto quantity = PitchMessage::parse_uint32(Beam::out(cursor));
    auto price = PitchMessage::parse_price(Beam::out(cursor));
    m_feed_client->modify_order_size(order_id, quantity, timestamp);
    m_feed_client->modify_order_price(order_id, price, timestamp);
    if(m_config.m_is_time_and_sale_feed) {
      if(auto order_entry = Beam::lookup(m_order_entries, order_id)) {
        order_entry->m_price = price;
      }
    }
    if(m_config.m_is_logging_messages) {
      std::cout << timestamp << ',' << message.m_type << ',' << order_id <<
        ',' << quantity << ',' << price << std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::handle_delete_order_message(
      const PitchMessage& message) {
    auto cursor = message.m_payload;
    auto timestamp = PitchMessage::parse_timestamp(Beam::out(cursor));
    auto order_id =
      std::to_string(PitchMessage::parse_uint64(Beam::out(cursor)));
    m_feed_client->remove_order(order_id, timestamp);
    if(m_config.m_is_logging_messages) {
      std::cout << timestamp << ',' << message.m_type << ',' << order_id <<
        std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::handle_trade_message(
      const PitchMessage& message) {
    auto cursor = message.m_payload;
    auto timestamp = PitchMessage::parse_timestamp(Beam::out(cursor));
    auto symbol = PitchMessage::parse_alphanumeric(6, Beam::out(cursor));
    auto quantity = PitchMessage::parse_uint32(Beam::out(cursor));
    if(quantity == 0) {
      return;
    }
    auto price = PitchMessage::parse_price(Beam::out(cursor));
    auto execution_id = PitchMessage::parse_uint64(Beam::out(cursor));
    auto order_id = PitchMessage::parse_uint64(Beam::out(cursor));
    auto contra_order_id = PitchMessage::parse_uint64(Beam::out(cursor));
    auto buyer_mpid = parse_mpid(Beam::out(cursor));
    auto seller_mpid = parse_mpid(Beam::out(cursor));
    auto security = Security(symbol, m_config.m_primary_venue);
    auto condition = TimeAndSale::Condition();
    condition.m_code = "@";
    auto time_and_sale = TimeAndSale(
      timestamp, price, quantity, std::move(condition), m_config.m_mpid,
      std::move(buyer_mpid), std::move(seller_mpid));
    m_feed_client->publish(
      SecurityTimeAndSale(std::move(time_and_sale), security));
    if(m_config.m_is_logging_messages) {
      std::cout << timestamp << ',' << message.m_type << ',' << symbol << ',' <<
        quantity << ',' << price << std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::dispatch(const PitchMessage& message) {
    static const auto ADD_ORDER_MESSAGE = 0x37;
    static const auto ORDER_EXECUTED_MESSAGE = 0x38;
    static const auto REDUCE_SIZE_MESSAGE = 0x39;
    static const auto MODIFY_ORDER_MESSAGE = 0x3A;
    static const auto DELETE_ORDER_MESSAGE = 0x3C;
    static const auto TRADE_MESSAGE = 0x3D;
    if(message.m_type == ADD_ORDER_MESSAGE) {
      handle_add_order_message(message);
    } else if(message.m_type == ORDER_EXECUTED_MESSAGE) {
      handle_order_executed_message(message);
    } else if(message.m_type == REDUCE_SIZE_MESSAGE) {
      handle_reduce_size_message(message);
    } else if(message.m_type == MODIFY_ORDER_MESSAGE) {
      handle_modify_order_message(message);
    } else if(message.m_type == DELETE_ORDER_MESSAGE) {
      handle_delete_order_message(message);
    } else if(message.m_type == TRADE_MESSAGE) {
      handle_trade_message(message);
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::read_loop() {
    while(true) {
      try {
        dispatch(m_protocol_client->read());
      } catch(const Beam::EndOfFileException&) {
        break;
      }
    }
  }
}

#endif
