#ifndef OTC_LINK_MARKET_DATA_FEED_CLIENT_HPP
#define OTC_LINK_MARKET_DATA_FEED_CLIENT_HPP
#include <cstdint>
#include <unordered_map>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/endian/conversion.hpp>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkClient.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkConfiguration.hpp"

namespace Nexus {

  /**
   * Parses packets from the OTC Link data feed.
   * @tparam M The type of MarketDataFeedClient used to update the
   *         MarketDataServer.
   * @tparam O The type of OtcLinkClient receiving messages.
   */
  template<typename M, typename O>
  class OtcLinkMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::dereference_t<M>;

      /** The type of client receiving OTC Link messages. */
      using OtcLinkClient = Beam::dereference_t<O>;

      /**
       * Constructs an OtcLinkMarketDataFeedClient.
       * @param configuration The OtcLinkConfiguration used to parse messages.
       * @param feed_client Initializes the MarketDataFeedClient.
       * @param otc_link_client The OtcLinkClient receiving messages.
       */
      template<Beam::Initializes<M> MF, Beam::Initializes<O> OF>
      OtcLinkMarketDataFeedClient(OtcLinkConfiguration configuration,
        MF&& feed_client, OF&& otc_link_client);

      ~OtcLinkMarketDataFeedClient();

      void close();

    private:
      struct BookQuoteEntry {
        Security m_security;
        BookQuote m_bid;
        BookQuote m_ask;
      };
      struct BboQuoteEntry {
        Security m_security;
        BboQuote m_bbo;
      };
      OtcLinkConfiguration m_configuration;
      Beam::local_ptr_t<M> m_feed_client;
      Beam::local_ptr_t<O> m_otc_link_client;
      std::unordered_map<std::uint32_t, BookQuoteEntry> m_book_quotes;
      std::unordered_map<std::uint32_t, BboQuoteEntry> m_bbo_quotes;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      OtcLinkMarketDataFeedClient(const OtcLinkMarketDataFeedClient&) = delete;
      OtcLinkMarketDataFeedClient& operator =(
        const OtcLinkMarketDataFeedClient&) = delete;
      static std::uint8_t parse_byte(const char* data);
      static std::uint32_t parse_uint32(const char* data);
      static Quantity parse_quantity(const char* data);
      static Money parse_money(const char* data);
      static boost::posix_time::ptime parse_timestamp(const char* data);
      void parse_quote_message(const OtcLinkMessage& message);
      void parse_quote_update_message(const OtcLinkMessage& message);
      void parse_inside_message(const OtcLinkMessage& message);
      void parse_inside_update_message(const OtcLinkMessage& message);
      void parse_trade_message(const OtcLinkMessage& message);
      void read_loop();
  };

  template<typename M, typename O>
  template<Beam::Initializes<M> MF, Beam::Initializes<O> OF>
  OtcLinkMarketDataFeedClient<M, O>::OtcLinkMarketDataFeedClient(
    OtcLinkConfiguration configuration,  MF&& feed_client, OF&& otc_link_client)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_configuration(std::move(configuration)),
            m_feed_client(std::forward<MF>(feed_client)),
            m_otc_link_client(std::forward<OF>(otc_link_client)),
            m_read_loop(Beam::spawn(
              std::bind(&OtcLinkMarketDataFeedClient::read_loop, this))) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Failed to initialize the OTC Link market data feed client."));
  }

  template<typename M, typename O>
  OtcLinkMarketDataFeedClient<M, O>::~OtcLinkMarketDataFeedClient() {
    close();
  }

  template<typename M, typename O>
  void OtcLinkMarketDataFeedClient<M, O>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_otc_link_client->close();
    m_feed_client->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename M, typename O>
  std::uint8_t OtcLinkMarketDataFeedClient<M, O>::parse_byte(const char* data) {
    return *reinterpret_cast<const std::uint8_t*>(data);
  }

  template<typename M, typename O>
  std::uint32_t OtcLinkMarketDataFeedClient<M, O>::parse_uint32(
      const char* data) {
    return boost::endian::big_to_native(
      *reinterpret_cast<const std::uint32_t*>(data));
  }

  template<typename M, typename O>
  Quantity OtcLinkMarketDataFeedClient<M, O>::parse_quantity(const char* data) {
    return Quantity(parse_uint32(data));
  }

  template<typename M, typename O>
  Money OtcLinkMarketDataFeedClient<M, O>::parse_money(const char* data) {
    auto value = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint64_t*>(data));
    return Money(Quantity(value) / 1000000);
  }

  template<typename M, typename O>
  boost::posix_time::ptime OtcLinkMarketDataFeedClient<M, O>::parse_timestamp(
      const char* data) {
    auto milliseconds = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint64_t*>(data));
    return boost::posix_time::ptime(
      boost::gregorian::date(1970, 1, 1),
      boost::posix_time::milliseconds(milliseconds));
  }

  template<typename M, typename O>
  void OtcLinkMarketDataFeedClient<M, O>::parse_quote_message(
      const OtcLinkMessage& message) {
    static const auto QUOTE_ACTION_ADD = std::uint8_t(0x02);
    static const auto QUOTE_ACTION_DELETE = std::uint8_t(0x03);
    static const auto QUOTE_ACTION_SPIN = std::uint8_t(0x04);
    auto data = message.m_payload;
    auto quote_id = parse_uint32(data + 4);
    auto quote_action = parse_byte(data + 8);
    if(quote_action == QUOTE_ACTION_DELETE) {
      auto i = m_book_quotes.find(quote_id);
      if(i == m_book_quotes.end()) {
        if(m_configuration.m_is_logging_messages) {
          std::cout << boost::posix_time::microsec_clock::universal_time() <<
            " Delete: quote_id " << quote_id << " not found" << std::endl;
        }
        return;
      }
      auto& stored = i->second;
      if(stored.m_bid.m_quote.m_price != Money::ZERO) {
        auto bid = stored.m_bid;
        bid.m_quote.m_size = 0;
        bid.m_timestamp = parse_timestamp(data + 52);
        if(m_configuration.m_is_logging_messages) {
          std::cout << boost::posix_time::microsec_clock::universal_time() <<
            " Delete: " << SecurityBookQuote(bid, stored.m_security) <<
            std::endl;
        }
        m_feed_client->publish(SecurityBookQuote(bid, stored.m_security));
      }
      if(stored.m_ask.m_quote.m_price != Money::ZERO) {
        auto ask = stored.m_ask;
        ask.m_quote.m_size = 0;
        ask.m_timestamp = parse_timestamp(data + 31);
        if(m_configuration.m_is_logging_messages) {
          std::cout << boost::posix_time::microsec_clock::universal_time() <<
            " Delete: " << SecurityBookQuote(ask, stored.m_security) <<
            std::endl;
        }
        m_feed_client->publish(SecurityBookQuote(ask, stored.m_security));
      }
      m_book_quotes.erase(i);
      return;
    }
    auto security_id = parse_uint32(data + 10);
    auto mpid = std::string(data + 14, 4);
    auto security =
      Security(std::to_string(security_id), m_configuration.m_venue);
    auto bid_price = parse_money(data + 39);
    auto bid = BookQuote();
    if(bid_price != Money::ZERO) {
      auto bid_size = parse_quantity(data + 47);
      auto bid_timestamp = parse_timestamp(data + 52);
      bid = BookQuote(mpid, true, m_configuration.m_venue,
        make_bid(bid_price, bid_size), bid_timestamp);
      if(m_configuration.m_is_logging_messages) {
        std::cout << boost::posix_time::microsec_clock::universal_time() <<
          " Add: " << SecurityBookQuote(bid, security) << std::endl;
      }
      m_feed_client->publish(SecurityBookQuote(bid, security));
    }
    auto ask_price = parse_money(data + 18);
    auto ask = BookQuote();
    if(ask_price != Money::ZERO) {
      auto ask_size = parse_quantity(data + 26);
      auto ask_timestamp = parse_timestamp(data + 31);
      ask = BookQuote(mpid, true, m_configuration.m_venue,
        make_ask(ask_price, ask_size), ask_timestamp);
      if(m_configuration.m_is_logging_messages) {
        std::cout << boost::posix_time::microsec_clock::universal_time() <<
          " Add: " << SecurityBookQuote(ask, security) << std::endl;
      }
      m_feed_client->publish(SecurityBookQuote(ask, security));
    }
    m_book_quotes.insert(
      std::pair(quote_id, BookQuoteEntry(security, bid, ask)));
  }

  template<typename M, typename O>
  void OtcLinkMarketDataFeedClient<M, O>::parse_quote_update_message(
      const OtcLinkMessage& message) {
    static const auto UPDATE_SIDE_ASK = std::uint8_t(0x01);
    auto data = message.m_payload;
    auto quote_id = parse_uint32(data + 4);
    auto i = m_book_quotes.find(quote_id);
    if(i == m_book_quotes.end()) {
      if(m_configuration.m_is_logging_messages) {
        std::cout << boost::posix_time::microsec_clock::universal_time() <<
          " Update: quote_id " << quote_id << " not found" << std::endl;
      }
      return;
    }
    auto quote_flags = parse_byte(data + 8);
    auto is_ask_update = (quote_flags & UPDATE_SIDE_ASK) != 0;
    auto price = parse_money(data + 9);
    auto size = parse_quantity(data + 17);
    auto timestamp = parse_timestamp(data + 22);
    auto& stored = i->second;
    auto& quote = [&] () -> auto& {
      if(is_ask_update) {
        return stored.m_ask;
      }
      return stored.m_bid;
    }();
    quote.m_quote.m_price = price;
    quote.m_quote.m_size = size;
    quote.m_timestamp = timestamp;
    if(m_configuration.m_is_logging_messages) {
      std::cout << boost::posix_time::microsec_clock::universal_time() <<
        " Update: " << SecurityBookQuote(quote, stored.m_security) << std::endl;
    }
    m_feed_client->publish(SecurityBookQuote(quote, stored.m_security));
  }

  template<typename M, typename O>
  void OtcLinkMarketDataFeedClient<M, O>::parse_inside_message(
      const OtcLinkMessage& message) {
    static const auto INSIDE_ACTION_ADD = std::uint8_t(0x02);
    static const auto INSIDE_ACTION_DELETE = std::uint8_t(0x03);
    static const auto INSIDE_ACTION_SPIN = std::uint8_t(0x04);
    auto data = message.m_payload;
    auto inside_id = parse_uint32(data + 4);
    auto inside_action = parse_byte(data + 8);
    if(inside_action == INSIDE_ACTION_DELETE) {
      m_bbo_quotes.erase(inside_id);
      return;
    }
    auto security_id = parse_uint32(data + 10);
    auto security =
      Security(std::to_string(security_id), m_configuration.m_venue);
    auto ask_price = parse_money(data + 14);
    auto ask_size = parse_quantity(data + 22);
    auto ask_timestamp = parse_timestamp(data + 26);
    auto bid_price = parse_money(data + 34);
    auto bid_size = parse_quantity(data + 42);
    auto bid_timestamp = parse_timestamp(data + 46);
    auto timestamp = std::max(bid_timestamp, ask_timestamp);
    auto bid = make_bid(bid_price, bid_size);
    auto ask = make_ask(ask_price, ask_size);
    auto bbo = BboQuote(bid, ask, timestamp);
    m_bbo_quotes.insert(std::pair(inside_id, BboQuoteEntry(security, bbo)));
    if(m_configuration.m_is_logging_messages) {
      std::cout << boost::posix_time::microsec_clock::universal_time() <<
        " Inside add: " << SecurityBboQuote(bbo, security) << std::endl;
    }
    m_feed_client->publish(SecurityBboQuote(bbo, security));
  }

  template<typename M, typename O>
  void OtcLinkMarketDataFeedClient<M, O>::parse_inside_update_message(
      const OtcLinkMessage& message) {
    static const auto UPDATE_SIDE_ASK = std::uint8_t(0x01);
    auto data = message.m_payload;
    auto inside_id = parse_uint32(data + 4);
    auto i = m_bbo_quotes.find(inside_id);
    if(i == m_bbo_quotes.end()) {
      if(m_configuration.m_is_logging_messages) {
        std::cout << boost::posix_time::microsec_clock::universal_time() <<
          " Inside update: inside_id " << inside_id << " not found" <<
          std::endl;
      }
      return;
    }
    auto quote_flags = parse_byte(data + 8);
    auto is_ask_update = (quote_flags & UPDATE_SIDE_ASK) != 0;
    auto price = parse_money(data + 9);
    auto size = parse_quantity(data + 17);
    auto timestamp = parse_timestamp(data + 21);
    auto& stored = i->second;
    if(is_ask_update) {
      stored.m_bbo.m_ask.m_price = price;
      stored.m_bbo.m_ask.m_size = size;
    } else {
      stored.m_bbo.m_bid.m_price = price;
      stored.m_bbo.m_bid.m_size = size;
    }
    stored.m_bbo.m_timestamp = timestamp;
    if(stored.m_bbo.m_bid.m_price == Money::ZERO ||
        stored.m_bbo.m_ask.m_price == Money::ZERO) {
      if(m_configuration.m_is_logging_messages) {
        std::cout << boost::posix_time::microsec_clock::universal_time() <<
          " Inside update: inside_id " << inside_id <<
          " bid or ask price is zero" << std::endl;
      }
      return;
    }
    if(m_configuration.m_is_logging_messages) {
      std::cout << boost::posix_time::microsec_clock::universal_time() <<
        " Inside update: " <<
        SecurityBboQuote(stored.m_bbo, stored.m_security) << std::endl;
    }
    m_feed_client->publish(SecurityBboQuote(stored.m_bbo, stored.m_security));
  }

  template<typename M, typename O>
  void OtcLinkMarketDataFeedClient<M, O>::parse_trade_message(
      const OtcLinkMessage& message) {
    auto data = message.m_payload;
    auto security_id = parse_uint32(data + 10);
    auto venue = std::string(data + 15, 3);
    auto price = parse_money(data + 23);
    auto size = parse_quantity(data + 31);
    auto timestamp = parse_timestamp(data + 35);
    auto security =
      Security(std::to_string(security_id), m_configuration.m_venue);
    auto condition = TimeAndSale::Condition(
      TimeAndSale::Condition::Type::REGULAR, "@");
    auto time_and_sale = TimeAndSale(
      timestamp, price, size, condition, venue, std::string(), std::string());
    if(m_configuration.m_is_logging_messages) {
      std::cout << boost::posix_time::microsec_clock::universal_time() <<
        " Trade: " << SecurityTimeAndSale(time_and_sale, security) << std::endl;
    }
    m_feed_client->publish(SecurityTimeAndSale(time_and_sale, security));
  }

  template<typename M, typename O>
  void OtcLinkMarketDataFeedClient<M, O>::read_loop() {
    while(true) {
      auto message = OtcLinkMessage();
      try {
        message = m_otc_link_client->read();
      } catch(const Beam::EndOfFileException&) {
        break;
      }
      if(message.m_type == OtcLinkMessage::Type::QUOTE) {
        parse_quote_message(message);
      } else if(message.m_type == OtcLinkMessage::Type::QUOTE_UPDATE) {
        parse_quote_update_message(message);
      } else if(message.m_type == OtcLinkMessage::Type::INSIDE) {
        parse_inside_message(message);
      } else if(message.m_type == OtcLinkMessage::Type::INSIDE_UPDATE) {
        parse_inside_update_message(message);
      } else if(message.m_type == OtcLinkMessage::Type::TRADE) {
        parse_trade_message(message);
      } else if(m_configuration.m_is_logging_messages) {
        std::cout << boost::posix_time::microsec_clock::universal_time() <<
          " Unhandled message type: " << static_cast<int>(message.m_type) <<
          std::endl;
      }
    }
  }
}

#endif
