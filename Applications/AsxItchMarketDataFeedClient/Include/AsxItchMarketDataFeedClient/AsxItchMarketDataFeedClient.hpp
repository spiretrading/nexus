#ifndef NEXUS_ASX_ITCH_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_ASX_ITCH_MARKET_DATA_FEED_CLIENT_HPP
#include <cstdint>
#include <string>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/IOException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/endian.hpp>
#include "AsxItchMarketDataFeedClient/AsxItchConfiguration.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MoldUdp64/MoldUdp64Client.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpClient.hpp"

namespace Nexus {

  /** The type of product. */
  enum class ProductType {

    /** An option. */
    OPTION,

    /** A future. */
    FUTURE,

    /** A cash equity. */
    EQUITY,
  };

  /** Stores info needed to maintain a Security's order book. */
  struct OrderBookDirectory {

    /** The order book id. */
    std::uint32_t m_id;

    /** The type of product. */
    ProductType m_product_type;

    /** Details about the Security represented by this order book. */
    SecurityInfo m_security;

    /** The book's currency. */
    CurrencyId m_currency;

    /** The number of decimal places used in the book's price. */
    std::uint16_t m_price_decimal_places;

    /** The number of decimal places used in the book's nominal value. */
    std::uint16_t m_value_decimal_places;

    /** The size of an odd-lot. */
    std::uint32_t m_odd_lot_size;

    /** The size of a block-lot. */
    std::uint64_t m_block_lot_size;
  };

  /**
   * Parses packets from the ASX ITCH feed.
   * @param <M> The type of MarketDataFeedClient used to update the
   *            MarketDataServer.
   * @param <I> The type of client receiving ITCH messages.
   * @param <G> The type of client connecting to Glimpse.
   */
  template<typename M, typename I, typename G>
  class AsxItchMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::dereference_t<M>;

      /** The type of client receiving ITCH messages. */
      using ItchClient = Beam::dereference_t<I>;

      /** The type of client connecting to Glimpse. */
      using GlimpseClient = Beam::dereference_t<G>;

      /**
       * Constructs a AsxItchMarketDataFeedClient.
       * @param config The configuration to use.
       * @param market_data_feed_client Initializes the MarketDataFeedClient.
       * @param itch_client The client receiving ITCH messages.
       * @param glimpse_client The client connecting to Glimpse.
       */
      template<typename MF, typename IF, typename GF>
      AsxItchMarketDataFeedClient(const AsxItchConfiguration& config,
        MF&& market_data_feed_client, IF&& itch_client, GF&& glimpse_client);

      ~AsxItchMarketDataFeedClient();

      void close();

    private:
      struct OrderEntry {
        std::string m_mpid;
        Money m_price;
        Quantity m_remaining_quantity;
      };
      struct PriceLevel {
        Money m_price;
        Quantity m_quantity;
      };
      struct BboEntry {
        std::vector<PriceLevel> m_asks;
        std::vector<PriceLevel> m_bids;
      };
      AsxItchConfiguration m_config;
      Beam::local_ptr_t<M> m_market_data_feed_client;
      Beam::local_ptr_t<I> m_itch_client;
      Beam::local_ptr_t<G> m_glimpse_client;
      boost::posix_time::ptime m_last_time_point;
      std::unordered_map<std::uint32_t, OrderBookDirectory>
        m_order_book_directories;
      std::unordered_map<Security, BboEntry> m_bbo_entries;
      std::unordered_map<std::string, OrderEntry> m_order_entries;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      AsxItchMarketDataFeedClient(const AsxItchMarketDataFeedClient&) = delete;
      AsxItchMarketDataFeedClient& operator =(
        const AsxItchMarketDataFeedClient&) = delete;
      boost::posix_time::ptime
        parse_timestamp(Beam::Out<const char*> cursor) const;
      std::uint8_t parse_char(Beam::Out<const char*> cursor) const;
      std::uint8_t parse_int8(Beam::Out<const char*> cursor) const;
      std::uint16_t parse_int16(Beam::Out<const char*> cursor) const;
      std::uint32_t parse_int32(Beam::Out<const char*> cursor) const;
      std::uint64_t parse_int64(Beam::Out<const char*> cursor) const;
      std::string
        parse_alpha(std::size_t size, Beam::Out<const char*> cursor) const;
      Side parse_side(Beam::Out<const char*> cursor) const;
      Money parse_price(const OrderBookDirectory& directory,
        Beam::Out<const char*> cursor) const;
      std::string parse_mpid(Beam::Out<const char*> cursor) const;
      std::tuple<std::string, std::string> parse_buyer_seller_mpids(
        Side side, Beam::Out<const char*> cursor) const;
      std::string build_order_key(const Security& security, Side side,
        std::uint64_t order_id) const;
      void update_bbo(const Security& security, Side side, Money price,
        Quantity delta, boost::posix_time::ptime timestamp);
      void handle_seconds_message(const MoldUdp64Message& message);
      void handle_add_order_message(
        bool is_anonymous, const MoldUdp64Message& message);
      void handle_order_executed_message(const MoldUdp64Message& message);
      void handle_order_executed_at_price_message(
        const MoldUdp64Message& message);
      void handle_order_replaced_message(const MoldUdp64Message& message);
      void handle_order_delete_message(const MoldUdp64Message& message);
      void handle_trade_message(const MoldUdp64Message& message);
      void handle_order_book_directory_message(const MoldUdp64Message& message);
      void dispatch(const MoldUdp64Message& message);
      void read_loop();
  };

  template<typename M, typename I, typename G>
  template<typename MF, typename IF, typename GF>
  AsxItchMarketDataFeedClient<M, I, G>::AsxItchMarketDataFeedClient(
      const AsxItchConfiguration& config, MF&& market_data_feed_client,
      IF&& itch_client, GF&& glimpse_client)
      try : m_config(config),
            m_market_data_feed_client(
              std::forward<MF>(market_data_feed_client)),
            m_itch_client(std::forward<IF>(itch_client)),
            m_glimpse_client(std::forward<GF>(glimpse_client)),
            m_last_time_point(boost::posix_time::not_a_date_time),
            m_read_loop(Beam::spawn(
              std::bind(&AsxItchMarketDataFeedClient::read_loop, this))) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Failed to initialize the ASX ITCH market data feed client."));
  }

  template<typename M, typename I, typename G>
  AsxItchMarketDataFeedClient<M, I, G>::~AsxItchMarketDataFeedClient() {
   close();
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_glimpse_client->close();
    m_itch_client->close();
    m_market_data_feed_client->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename M, typename I, typename G>
  boost::posix_time::ptime
      AsxItchMarketDataFeedClient<M, I, G>::parse_timestamp(
        Beam::Out<const char*> cursor) const {
    auto nanoseconds = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint32_t*>(*cursor));
    *cursor += sizeof(std::uint32_t);
    if(m_last_time_point == boost::posix_time::not_a_date_time) {
      return m_last_time_point;
    }
    return m_last_time_point +
      boost::posix_time::microseconds(nanoseconds / 1000);
  }

  template<typename M, typename I, typename G>
  std::uint8_t AsxItchMarketDataFeedClient<M, I, G>::parse_char(
      Beam::Out<const char*> cursor) const {
    auto result = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint8_t*>(*cursor));
    *cursor += sizeof(std::uint8_t);
    return result;
  }

  template<typename M, typename I, typename G>
  std::uint8_t AsxItchMarketDataFeedClient<M, I, G>::parse_int8(
      Beam::Out<const char*> cursor) const {
    auto result = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint8_t*>(*cursor));
    *cursor += sizeof(std::uint8_t);
    return result;
  }

  template<typename M, typename I, typename G>
  std::uint16_t AsxItchMarketDataFeedClient<M, I, G>::parse_int16(
      Beam::Out<const char*> cursor) const {
    auto result = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint16_t*>(*cursor));
    *cursor += sizeof(std::uint16_t);
    return result;
  }

  template<typename M, typename I, typename G>
  std::uint32_t AsxItchMarketDataFeedClient<M, I, G>::parse_int32(
      Beam::Out<const char*> cursor) const {
    auto result = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint32_t*>(*cursor));
    *cursor += sizeof(std::uint32_t);
    return result;
  }

  template<typename M, typename I, typename G>
  std::uint64_t AsxItchMarketDataFeedClient<M, I, G>::parse_int64(
      Beam::Out<const char*> cursor) const {
    auto result = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint64_t*>(*cursor));
    *cursor += sizeof(std::uint64_t);
    return result;
  }

  template<typename M, typename I, typename G>
  std::string AsxItchMarketDataFeedClient<M, I, G>::parse_alpha(
      std::size_t size, Beam::Out<const char*> cursor) const {
    if(size == 0) {
      return std::string();
    }
    auto last_character = *cursor + size - 1;
    while(true) {
      if(*last_character == ' ') {
        if(last_character == *cursor) {
          *cursor += size;
          return std::string();
        } else {
          --last_character;
        }
      } else {
        break;
      }
    }
    auto result = std::string(*cursor, last_character + 1);
    *cursor += size;
    return result;
  }

  template<typename M, typename I, typename G>
  Side AsxItchMarketDataFeedClient<M, I, G>::parse_side(
      Beam::Out<const char*> cursor) const {
    auto s = parse_char(Beam::out(cursor));
    if(s == 'S') {
      return Side::ASK;
    } else if(s == 'B') {
      return Side::BID;
    }
    return Side::NONE;
  }

  template<typename M, typename I, typename G>
  Money AsxItchMarketDataFeedClient<M, I, G>::parse_price(
      const OrderBookDirectory& directory,
      Beam::Out<const char*> cursor) const {
    auto power_of_ten = [] (std::uint16_t exponent) {
      auto result = Quantity(1);
      for(auto i = std::uint16_t(0); i < exponent; ++i) {
        result *= 10;
      }
      return result;
    };
    auto price = Money(parse_int32(Beam::out(cursor)) /
      (100 * power_of_ten(directory.m_price_decimal_places)));
    return price;
  }

  template<typename M, typename I, typename G>
  std::string AsxItchMarketDataFeedClient<M, I, G>::parse_mpid(
      Beam::Out<const char*> cursor) const {
    auto mpid = parse_alpha(7, Beam::out(cursor));
    if(mpid.empty()) {
      return "AU000";
    }
    return mpid;
  }

  template<typename M, typename I, typename G>
  std::tuple<std::string, std::string>
    AsxItchMarketDataFeedClient<M, I, G>::parse_buyer_seller_mpids(
      Side side, Beam::Out<const char*> cursor) const {
    auto owner_mpid = parse_mpid(Beam::out(cursor));
    auto counter_mpid = parse_mpid(Beam::out(cursor));
    if(side == Side::BID) {
      return std::tuple(std::move(owner_mpid), std::move(counter_mpid));
    }
    return std::tuple(std::move(counter_mpid), std::move(owner_mpid));
  }

  template<typename M, typename I, typename G>
  std::string AsxItchMarketDataFeedClient<M, I, G>::build_order_key(
      const Security& security, Side side, std::uint64_t order_id) const {
    auto result = security.get_symbol();
    result += '-';
    if(side == Side::ASK) {
      result += 'A';
    } else {
      result += 'B';
    }
    result += '-';
    result += std::to_string(order_id);
    return result;
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::handle_seconds_message(
      const MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto epoch_time = static_cast<std::time_t>(boost::endian::big_to_native(
      *reinterpret_cast<const std::uint32_t*>(cursor)));
    m_last_time_point = boost::posix_time::from_time_t(epoch_time);
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::update_bbo(
      const Security& security, Side side, Money price, Quantity delta,
      boost::posix_time::ptime timestamp) {
    auto& bbo_entry = m_bbo_entries[security];
    auto& levels = pick(side, bbo_entry.m_asks, bbo_entry.m_bids);
    auto i = std::lower_bound(
      levels.begin(), levels.end(), price, [&] (const auto& lhs, auto rhs) {
        if(side == Side::ASK) {
          return lhs.m_price < rhs;
        } else {
          return lhs.m_price > rhs;
        }
      });
    if(i == levels.end() || i->m_price != price) {
      if(delta <= 0) {
        return;
      }
      auto level = PriceLevel();
      level.m_price = price;
      level.m_quantity = delta;
      i = levels.insert(i, level);
    } else {
      auto& level = *i;
      level.m_quantity += delta;
      if(level.m_quantity <= 0) {
        i = levels.erase(i);
      }
    }
    if(i == levels.begin()) {
      auto ask = Quote();
      ask.m_side = Side::ASK;
      if(bbo_entry.m_asks.empty()) {
        ask.m_price = Money::ZERO;
        ask.m_size = 0;
      } else {
        ask.m_price = bbo_entry.m_asks.front().m_price;
        ask.m_size = bbo_entry.m_asks.front().m_quantity;
      }
      auto bid = Quote();
      bid.m_side = Side::BID;
      if(bbo_entry.m_bids.empty()) {
        bid.m_price = Money::ZERO;
        bid.m_size = 0;
      } else {
        bid.m_price = bbo_entry.m_bids.front().m_price;
        bid.m_size = bbo_entry.m_bids.front().m_quantity;
      }
      auto bbo = BboQuote(bid, ask, timestamp);
      m_market_data_feed_client->publish(SecurityBboQuote(bbo, security));
    }
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::handle_add_order_message(
      bool is_anonymous, const MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto timestamp = parse_timestamp(Beam::out(cursor));
    auto order_id = parse_int64(Beam::out(cursor));
    auto order_book_id = parse_int32(Beam::out(cursor));
    auto directory = Beam::lookup(m_order_book_directories, order_book_id);
    if(!directory) {
      return;
    }
    auto side = parse_side(Beam::out(cursor));
    auto position = parse_int32(Beam::out(cursor));
    auto quantity = parse_int64(Beam::out(cursor));
    if(quantity == 0) {
      return;
    }
    auto price = parse_price(*directory, Beam::out(cursor));
    auto type = parse_int16(Beam::out(cursor));
    auto lot = parse_int8(Beam::out(cursor));
    auto mpid = [&] {
      if(is_anonymous) {
        return std::string("AU000");
      } else {
        return parse_mpid(Beam::out(cursor));
      }
    }();
    auto order_key =
      build_order_key(directory->m_security.m_security, side, order_id);
    auto order_entry = OrderEntry();
    order_entry.m_mpid = mpid;
    order_entry.m_price = price;
    order_entry.m_remaining_quantity = quantity;
    m_order_entries[order_key] = order_entry;
    m_market_data_feed_client->add_order(directory->m_security.m_security,
      m_config.m_venue.m_venue, mpid, false, order_key, side, price, quantity,
      timestamp);
    update_bbo(
      directory->m_security.m_security, side, price, quantity, timestamp);
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::handle_order_executed_message(
      const MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto timestamp = parse_timestamp(Beam::out(cursor));
    auto order_id = parse_int64(Beam::out(cursor));
    auto order_book_id = parse_int32(Beam::out(cursor));
    auto directory = Beam::lookup(m_order_book_directories, order_book_id);
    if(!directory) {
      return;
    }
    auto side = parse_side(Beam::out(cursor));
    auto executed_quantity =
      static_cast<std::int64_t>(parse_int64(Beam::out(cursor)));
    auto order_key =
      build_order_key(directory->m_security.m_security, side, order_id);
    auto match_id = parse_alpha(12, Beam::out(cursor));
    auto [buyer_mpid, seller_mpid] =
      parse_buyer_seller_mpids(side, Beam::out(cursor));
    m_market_data_feed_client->offset_order_size(
      order_key, -executed_quantity, timestamp);
    if(auto order_entry = Beam::lookup(m_order_entries, order_key)) {
      order_entry->m_remaining_quantity -= executed_quantity;
      if(m_config.m_is_time_and_sale_feed) {
        auto condition = TimeAndSale::Condition();
        condition.m_code = "@";
        auto time_and_sale = TimeAndSale(timestamp, order_entry->m_price,
          executed_quantity, std::move(condition),
          m_config.m_venue.m_display_name, std::move(buyer_mpid),
          std::move(seller_mpid));
        m_market_data_feed_client->publish(SecurityTimeAndSale(
          std::move(time_and_sale), directory->m_security.m_security));
      }
      update_bbo(directory->m_security.m_security, side, order_entry->m_price,
        -executed_quantity, timestamp);
    }
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::handle_order_executed_at_price_message(
      const MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto timestamp = parse_timestamp(Beam::out(cursor));
    auto order_id = parse_int64(Beam::out(cursor));
    auto order_book_id = parse_int32(Beam::out(cursor));
    auto directory = Beam::lookup(m_order_book_directories, order_book_id);
    if(!directory) {
      return;
    }
    auto side = parse_side(Beam::out(cursor));
    auto executed_quantity =
      static_cast<std::int64_t>(parse_int64(Beam::out(cursor)));
    auto match_id = parse_alpha(12, Beam::out(cursor));
    auto [buyer_mpid, seller_mpid] =
      parse_buyer_seller_mpids(side, Beam::out(cursor));
    auto price = parse_price(*directory, Beam::out(cursor));
    auto at_cross = parse_char(Beam::out(cursor));
    auto printable = parse_char(Beam::out(cursor));
    auto order_key =
      build_order_key(directory->m_security.m_security, side, order_id);
    m_market_data_feed_client->offset_order_size(
      order_key, -executed_quantity, timestamp);
    if(auto order_entry = Beam::lookup(m_order_entries, order_key)) {
      order_entry->m_remaining_quantity -= executed_quantity;
      if(printable == 'Y' && m_config.m_is_time_and_sale_feed) {
        auto condition = TimeAndSale::Condition();
        condition.m_code = "@";
        auto time_and_sale = TimeAndSale(timestamp, price, executed_quantity,
          std::move(condition), m_config.m_venue.m_display_name,
          std::move(buyer_mpid), std::move(seller_mpid));
        m_market_data_feed_client->publish(SecurityTimeAndSale(
          std::move(time_and_sale), directory->m_security.m_security));
      }
      update_bbo(directory->m_security.m_security, side, order_entry->m_price,
        -executed_quantity, timestamp);
    }
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::handle_order_replaced_message(
      const MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto timestamp = parse_timestamp(Beam::out(cursor));
    auto order_id = parse_int64(Beam::out(cursor));
    auto order_book_id = parse_int32(Beam::out(cursor));
    auto directory = Beam::lookup(m_order_book_directories, order_book_id);
    if(!directory) {
      return;
    }
    auto side = parse_side(Beam::out(cursor));
    auto new_position = parse_int32(Beam::out(cursor));
    auto quantity = parse_int64(Beam::out(cursor));
    auto price = parse_price(*directory, Beam::out(cursor));
    auto type = parse_int16(Beam::out(cursor));
    auto order_key =
      build_order_key(directory->m_security.m_security, side, order_id);
    m_market_data_feed_client->remove_order(order_key, timestamp);
    auto order_entry = Beam::lookup(m_order_entries, order_key);
    if(!order_entry) {
      return;
    }
    update_bbo(directory->m_security.m_security, side, order_entry->m_price,
      -order_entry->m_remaining_quantity, timestamp);
    auto new_order_entry = *order_entry;
    new_order_entry.m_price = price;
    new_order_entry.m_remaining_quantity = quantity;
    m_order_entries[order_key] = new_order_entry;
    m_market_data_feed_client->add_order(directory->m_security.m_security,
      m_config.m_venue.m_venue, new_order_entry.m_mpid, false, order_key,
      side, price, quantity, timestamp);
    update_bbo(
      directory->m_security.m_security, side, price, quantity, timestamp);
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::handle_order_delete_message(
      const MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto timestamp = parse_timestamp(Beam::out(cursor));
    auto order_id = parse_int64(Beam::out(cursor));
    auto order_book_id = parse_int32(Beam::out(cursor));
    auto directory = Beam::lookup(m_order_book_directories, order_book_id);
    if(!directory) {
      return;
    }
    auto side = parse_side(Beam::out(cursor));
    auto order_key =
      build_order_key(directory->m_security.m_security, side, order_id);
    m_market_data_feed_client->remove_order(order_key, timestamp);
    auto order_entry = Beam::lookup(m_order_entries, order_key);
    if(!order_entry) {
      return;
    }
    update_bbo(directory->m_security.m_security, side, order_entry->m_price,
      -order_entry->m_remaining_quantity, timestamp);
    m_order_entries.erase(order_key);
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::handle_trade_message(
      const MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto timestamp = parse_timestamp(Beam::out(cursor));
    cursor += 12;
    auto side = parse_side(Beam::out(cursor));
    auto quantity = parse_int64(Beam::out(cursor));
    auto order_book_id = parse_int32(Beam::out(cursor));
    auto directory = Beam::lookup(m_order_book_directories, order_book_id);
    if(!directory) {
      return;
    }
    auto price = parse_price(*directory, Beam::out(cursor));
    auto [buyer_mpid, seller_mpid] =
      parse_buyer_seller_mpids(side, Beam::out(cursor));
    auto printable = parse_char(Beam::out(cursor));
    auto at_cross = parse_char(Beam::out(cursor));
    if(printable == 'Y' && m_config.m_is_time_and_sale_feed) {
      auto condition = TimeAndSale::Condition();
      condition.m_code = "@";
      auto time_and_sale = TimeAndSale(timestamp, price, quantity,
        std::move(condition), m_config.m_venue.m_display_name,
        std::move(buyer_mpid), std::move(seller_mpid));
      m_market_data_feed_client->publish(SecurityTimeAndSale(
        std::move(time_and_sale), directory->m_security.m_security));
    }
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::
      handle_order_book_directory_message(const MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto directory = OrderBookDirectory();
    auto timestamp = parse_timestamp(Beam::out(cursor));
    directory.m_id = parse_int32(Beam::out(cursor));
    auto symbol = parse_alpha(32, Beam::out(cursor));
    directory.m_security.m_security =
      Security(symbol, m_config.m_venue.m_venue);
    directory.m_security.m_name = parse_alpha(32, Beam::out(cursor));
    auto isin = parse_alpha(12, Beam::out(cursor));
    auto product_type = parse_int8(Beam::out(cursor));
    if(product_type == 1) {
      directory.m_product_type = ProductType::OPTION;
    } else if(product_type == 3) {
      directory.m_product_type = ProductType::FUTURE;
    } else if(product_type == 5) {
      directory.m_product_type = ProductType::EQUITY;
    }
    directory.m_currency =
      DEFAULT_CURRENCIES.from(parse_alpha(3, Beam::out(cursor))).m_id;
    directory.m_price_decimal_places = parse_int16(Beam::out(cursor));
    directory.m_value_decimal_places = parse_int16(Beam::out(cursor));
    directory.m_odd_lot_size = parse_int32(Beam::out(cursor));
    directory.m_security.m_board_lot = parse_int32(Beam::out(cursor));
    directory.m_block_lot_size = parse_int64(Beam::out(cursor));
    if(directory.m_product_type != ProductType::EQUITY) {
      return;
    }
    m_order_book_directories[directory.m_id] = directory;
    m_market_data_feed_client->add(directory.m_security);
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::dispatch(
      const MoldUdp64Message& message) {
    if(message.m_message_type == 'T') {
      handle_seconds_message(message);
    } else if(message.m_message_type == 'A') {
      handle_add_order_message(true, message);
    } else if(message.m_message_type == 'F') {
      handle_add_order_message(false, message);
    } else if(message.m_message_type == 'E') {
      handle_order_executed_message(message);
    } else if(message.m_message_type == 'C') {
      handle_order_executed_at_price_message(message);
    } else if(message.m_message_type == 'U') {
      handle_order_replaced_message(message);
    } else if(message.m_message_type == 'D') {
      handle_order_delete_message(message);
    } else if(message.m_message_type == 'P') {
      handle_trade_message(message);
    } else if(message.m_message_type == 'R') {
      handle_order_book_directory_message(message);
    }
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::read_loop() {
    auto last_sequence_number = std::uint64_t(-1);
    while(true) {
      auto packet = SoupBinTcpPacket();
      try {
        packet = m_glimpse_client->read();
      } catch(Beam::EndOfFileException&) {
        break;
      }
      if(packet.m_type == 'S') {
        auto message = MoldUdp64Message();
        message.m_length = packet.m_length - 1;
        message.m_message_type = packet.m_payload[0];
        message.m_data = &packet.m_payload[1];
        if(message.m_message_type == 'G') {
          auto cursor = message.m_data;
          last_sequence_number =
            parse_left_padded_numeric<std::uint64_t>(20, Beam::out(cursor));
          break;
        }
        dispatch(message);
      } else if(packet.m_type == 'Z') {
        break;
      }
    }
    m_glimpse_client->close();
    while(true) {
      try {
        auto sequence_number = std::uint64_t();
        auto message = m_itch_client->read(Beam::out(sequence_number));
        if(last_sequence_number != -1 &&
            sequence_number <= last_sequence_number) {
          continue;
        }
        if(last_sequence_number != -1 &&
            sequence_number > last_sequence_number + 1) {
          std::cout << "Packets dropped: " << (last_sequence_number + 1) <<
            " - " << (sequence_number - 1) << std::endl;
        }
        last_sequence_number = sequence_number;
        dispatch(message);
      } catch(Beam::EndOfFileException&) {
        break;
      }
    }
  }
}

#endif
