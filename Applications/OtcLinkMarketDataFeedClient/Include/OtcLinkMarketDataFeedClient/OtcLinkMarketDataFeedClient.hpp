#ifndef OTC_LINK_MARKET_DATA_FEED_CLIENT_HPP
#define OTC_LINK_MARKET_DATA_FEED_CLIENT_HPP
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
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
#include "OtcLinkMarketDataFeedClient/OtcLinkRecoveryClient.hpp"

namespace Nexus {

  /**
   * Parses packets from the OTC Link data feed.
   * @tparam M The type of MarketDataFeedClient used to update the
   *         MarketDataServer.
   * @tparam O The type of OtcLinkClient receiving messages.
   * @tparam R The type of OtcLinkRecoveryClient used for recovery requests.
   * @tparam S The type of OtcLinkClient used for recovery messages.
   */
  template<typename M, typename O, typename R, typename S>
  class OtcLinkMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::dereference_t<M>;

      /** The type of client receiving OTC Link messages. */
      using OtcLinkClient = Beam::dereference_t<O>;

      /** The type of client used for recovery requests. */
      using RecoveryClient = Beam::dereference_t<R>;

      /**
       * The type of function used to build OtcLinkClients used for recovery.
       */
      using SnapshotClientBuilder = std::function<S ()>;

      /**
       * Constructs an OtcLinkMarketDataFeedClient.
       * @param configuration The OtcLinkConfiguration used to parse messages.
       * @param symbol_table The path to a YAML document containing the mappings
       *        from ids to ticker symbols.
       * @param feed_client Initializes the MarketDataFeedClient.
       * @param otc_link_client The OtcLinkClient receiving messages.
       * @param recovery_client The RecoveryClient used for recovery requests.
       * @param snapshot_client_builder Builds snapshots when needed.
       */
      template<Beam::Initializes<M> MF, Beam::Initializes<O> OF,
        Beam::Initializes<R> RF>
      OtcLinkMarketDataFeedClient(OtcLinkConfiguration configuration,
        const std::filesystem::path& symbol_table, MF&& feed_client,
        OF&& otc_link_client, RF&& recovery_client,
        SnapshotClientBuilder snapshot_client_builder);

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
      std::filesystem::path m_symbol_table;
      Beam::local_ptr_t<M> m_feed_client;
      Beam::local_ptr_t<O> m_otc_link_client;
      Beam::local_ptr_t<R> m_recovery_client;
      SnapshotClientBuilder m_snapshot_client_builder;
      std::unordered_map<std::uint32_t, Security> m_securities;
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
      static std::string parse_alphanumeric(const char* data, std::size_t size);
      static boost::posix_time::ptime parse_timestamp(const char* data);
      const Security& find_security(std::uint32_t id) const;
      void parse_security_message(const OtcLinkMessage& message);
      void parse_quote_message(const OtcLinkMessage& message);
      void parse_quote_update_message(const OtcLinkMessage& message);
      void parse_inside_message(const OtcLinkMessage& message);
      void parse_inside_update_message(const OtcLinkMessage& message);
      void parse_trade_message(const OtcLinkMessage& message);
      void parse_message(const OtcLinkMessage& message);
      void load_securities();
      void save_securities();
      std::uint32_t initialize_snapshot();
      void read_loop();
  };

  template<typename M, typename O, typename R, typename S>
  template<Beam::Initializes<M> MF, Beam::Initializes<O> OF,
    Beam::Initializes<R> RF>
  OtcLinkMarketDataFeedClient<M, O, R, S>::OtcLinkMarketDataFeedClient(
    OtcLinkConfiguration configuration,
    const std::filesystem::path& symbol_table, MF&& feed_client,
    OF&& otc_link_client, RF&& recovery_client,
    SnapshotClientBuilder snapshot_client_builder)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_configuration(std::move(configuration)),
            m_symbol_table(symbol_table),
            m_feed_client(std::forward<MF>(feed_client)),
            m_otc_link_client(std::forward<OF>(otc_link_client)),
            m_recovery_client(std::forward<RF>(recovery_client)),
            m_snapshot_client_builder(std::move(snapshot_client_builder)),
            m_read_loop(Beam::spawn(
              std::bind(&OtcLinkMarketDataFeedClient::read_loop, this))) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Failed to initialize the OTC Link market data feed client."));
  }

  template<typename M, typename O, typename R, typename S>
  OtcLinkMarketDataFeedClient<M, O, R, S>::~OtcLinkMarketDataFeedClient() {
    close();
  }

  template<typename M, typename O, typename R, typename S>
  void OtcLinkMarketDataFeedClient<M, O, R, S>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_otc_link_client->close();
    m_feed_client->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename M, typename O, typename R, typename S>
  std::uint8_t OtcLinkMarketDataFeedClient<M, O, R, S>::parse_byte(
      const char* data) {
    return *reinterpret_cast<const std::uint8_t*>(data);
  }

  template<typename M, typename O, typename R, typename S>
  std::uint32_t OtcLinkMarketDataFeedClient<M, O, R, S>::parse_uint32(
      const char* data) {
    return boost::endian::big_to_native(
      *reinterpret_cast<const std::uint32_t*>(data));
  }

  template<typename M, typename O, typename R, typename S>
  Quantity OtcLinkMarketDataFeedClient<M, O, R, S>::parse_quantity(
      const char* data) {
    return Quantity(parse_uint32(data));
  }

  template<typename M, typename O, typename R, typename S>
  Money OtcLinkMarketDataFeedClient<M, O, R, S>::parse_money(const char* data) {
    auto value = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint64_t*>(data));
    return Money(Quantity(value) / 1000000);
  }

  template<typename M, typename O, typename R, typename S>
  std::string OtcLinkMarketDataFeedClient<M, O, R, S>::parse_alphanumeric(
      const char* data, std::size_t size) {
    auto value = std::string();
    for(auto i = std::size_t(0); i != size; ++i) {
      auto c = parse_byte(data + i);
      if(c == '\0') {
        break;
      }
      value += c;
    }
    return value;
  }

  template<typename M, typename O, typename R, typename S>
  boost::posix_time::ptime
      OtcLinkMarketDataFeedClient<M, O, R, S>::parse_timestamp(
        const char* data) {
    auto milliseconds = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint64_t*>(data));
    return boost::posix_time::ptime(
      boost::gregorian::date(1970, 1, 1),
      boost::posix_time::milliseconds(milliseconds));
  }

  template<typename M, typename O, typename R, typename S>
  const Security& OtcLinkMarketDataFeedClient<M, O, R, S>::find_security(
      std::uint32_t id) const {
    auto i = m_securities.find(id);
    if(i == m_securities.end()) {
      static const auto NONE = Security();
      return NONE;
    }
    return i->second;
  }

  template<typename M, typename O, typename R, typename S>
  void OtcLinkMarketDataFeedClient<M, O, R, S>::parse_security_message(
      const OtcLinkMessage& message) {
    static const auto SYMBOL_OFFSET = 4;
    static const auto SYMBOL_SIZE = 10;
    static const auto SECURITY_ACTION_OFFSET = 22;
    static const auto UPDATE_ACTION = std::uint8_t(0x01);
    static const auto ADD_ACTION = std::uint8_t(0x02);
    static const auto DELETE_ACTION = std::uint8_t(0x03);
    static const auto SPIN_ACTION = std::uint8_t(0x04);
    static const auto SECURITY_ID_OFFSET = 24;
    auto data = message.m_payload;
    auto symbol = parse_alphanumeric(data + SYMBOL_OFFSET, SYMBOL_SIZE);
    auto action = parse_byte(data + SECURITY_ACTION_OFFSET);
    auto id = parse_uint32(data + SECURITY_ID_OFFSET);
    if(action == ADD_ACTION ||
        action == UPDATE_ACTION || action == SPIN_ACTION) {
      if(!m_securities.contains(id)) {
        auto security = Security(std::move(symbol), m_configuration.m_venue);
        m_securities[id] = security;
        m_feed_client->add(SecurityInfo(
          security, boost::lexical_cast<std::string>(security), "", 100));
      }
    } else if(action == DELETE_ACTION) {
      m_securities.erase(id);
    }
  }

  template<typename M, typename O, typename R, typename S>
  void OtcLinkMarketDataFeedClient<M, O, R, S>::parse_quote_message(
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
    auto& security = find_security(security_id);
    if(!security) {
      if(m_configuration.m_is_logging_messages) {
        std::cout << boost::posix_time::microsec_clock::universal_time() <<
          " Security not found: " << security_id << std::endl;
      }
      return;
    }
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

  template<typename M, typename O, typename R, typename S>
  void OtcLinkMarketDataFeedClient<M, O, R, S>::parse_quote_update_message(
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

  template<typename M, typename O, typename R, typename S>
  void OtcLinkMarketDataFeedClient<M, O, R, S>::parse_inside_message(
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
    auto& security = find_security(security_id);
    if(!security) {
      if(m_configuration.m_is_logging_messages) {
        std::cout << boost::posix_time::microsec_clock::universal_time() <<
          " Security not found: " << security_id << std::endl;
      }
      return;
    }
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

  template<typename M, typename O, typename R, typename S>
  void OtcLinkMarketDataFeedClient<M, O, R, S>::parse_inside_update_message(
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

  template<typename M, typename O, typename R, typename S>
  void OtcLinkMarketDataFeedClient<M, O, R, S>::parse_trade_message(
      const OtcLinkMessage& message) {
    auto data = message.m_payload;
    auto security_id = parse_uint32(data + 10);
    auto venue = std::string(data + 15, 3);
    auto price = parse_money(data + 23);
    auto size = parse_quantity(data + 31);
    auto timestamp = parse_timestamp(data + 35);
    auto& security = find_security(security_id);
    if(!security) {
      if(m_configuration.m_is_logging_messages) {
        std::cout << boost::posix_time::microsec_clock::universal_time() <<
          " Security not found: " << security_id << std::endl;
      }
      return;
    }
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

  template<typename M, typename O, typename R, typename S>
  void OtcLinkMarketDataFeedClient<M, O, R, S>::parse_message(
      const OtcLinkMessage& message) {
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
    } else if(message.m_type == OtcLinkMessage::Type::SECURITY_REFERENCE) {
      parse_security_message(message);
    } else if(m_configuration.m_is_logging_messages) {
      std::cout << boost::posix_time::microsec_clock::universal_time() <<
        " Unhandled message type: " << static_cast<int>(message.m_type) <<
        std::endl;
    }
  }

  template<typename M, typename O, typename R, typename S>
  void OtcLinkMarketDataFeedClient<M, O, R, S>::load_securities() {
    auto symbol_table = std::ifstream(m_symbol_table);
    if(!symbol_table.good()) {
      std::cerr << "Unable to open ticker symbol mappings.\n";
      return;
    }
    try {
      auto yaml_symbols = YAML::Load(symbol_table);
      auto symbols_node = yaml_symbols["symbols"];
      if(!symbols_node) {
        std::cerr << "Symbols not found.\n";
        return;
      }
      for(const auto& node : symbols_node) {
        auto id = node["id"].as<int>();
        auto symbol = node["symbol"].as<std::string>();
        m_securities.insert(
          std::pair(id, Security(std::move(symbol), m_configuration.m_venue)));
      }
    } catch(const YAML::ParserException& e) {
      std::cerr << "Invalid YAML in file \"" << m_symbol_table <<
        "\" at line " << (e.mark.line + 1) << ", " << "column " <<
        (e.mark.column + 1) << ": " << e.msg << '\n';
    }
  }

  template<typename M, typename O, typename R, typename S>
  void OtcLinkMarketDataFeedClient<M, O, R, S>::save_securities() {
    auto node = YAML::Node();
    auto symbols = node["symbols"];
    symbols = YAML::Node(YAML::NodeType::Sequence);
    for(auto& [id, security] : m_securities) {
      auto entry = YAML::Node();
      entry["id"] = id;
      entry["symbol"] = security.get_symbol();
      symbols.push_back(entry);
    }
    auto symbol_table = std::ofstream(m_symbol_table, std::ios::trunc);
    if(!symbol_table.good()) {
      std::cerr << "Unable to save ticker symbol mappings.\n";
      return;
    }
    symbol_table << node;
  }

  template<typename M, typename O, typename R, typename S>
  std::uint32_t OtcLinkMarketDataFeedClient<M, O, R, S>::initialize_snapshot() {
    load_securities();
    if(m_configuration.m_recovery_channel !=
        OtcLinkChannelId::QUOTE_INSIDE_REAL_TIME) {
      return 0;
    }
    auto snapshot_client = m_snapshot_client_builder();
    {
      auto timer = Beam::LiveTimer(boost::posix_time::seconds(1));
      timer.start();
      timer.wait();
    }
    for(auto& security : m_securities | std::views::values) {
      m_feed_client->add(SecurityInfo(
        security, boost::lexical_cast<std::string>(security), "", 100));
    }
    auto result =
      m_recovery_client->request_snapshot(m_configuration.m_recovery_channel);
    if(result.m_response != OtcLinkReplayAckMessage::ResponseType::SUCCESS) {
      std::cout << boost::posix_time::microsec_clock::universal_time() <<
        " Snapshot request failed: " << result.m_text << std::endl;
      return 0;
    }
    auto last_sequence_number = std::uint32_t(0);
    auto received_start_of_spin = false;
    while(true) {
      auto message = OtcLinkMessage();
      try {
        message = snapshot_client->read();
      } catch(const Beam::EndOfFileException&) {
        break;
      }
      if(message.m_type == OtcLinkMessage::Type::START_OF_SPIN) {
        received_start_of_spin = true;
        if(m_configuration.m_is_logging_messages) {
          std::cout << boost::posix_time::microsec_clock::universal_time() <<
            " Start of spin" << std::endl;
        }
      } else if(message.m_type == OtcLinkMessage::Type::END_OF_SPIN) {
        const auto SPIN_TYPE_OFFSET = 4;
        const auto SPIN_LAST_SEQ_NUM_OFFSET = 17;
        const auto MARKET_DATA_SPIN_TYPE = 2;
        auto spin_type = parse_byte(message.m_payload + SPIN_TYPE_OFFSET);
        last_sequence_number =
          parse_uint32(message.m_payload + SPIN_LAST_SEQ_NUM_OFFSET);
        if(m_configuration.m_is_logging_messages) {
          std::cout << boost::posix_time::microsec_clock::universal_time() <<
            " End of spin, last sequence number: " << last_sequence_number <<
            std::endl;
        }
        if(spin_type == MARKET_DATA_SPIN_TYPE) {
          break;
        }
      } else if(received_start_of_spin) {
        parse_message(message);
      }
    }
    save_securities();
    return 0;
  }

  template<typename M, typename O, typename R, typename S>
  void OtcLinkMarketDataFeedClient<M, O, R, S>::read_loop() {
    auto last_sequence_number = initialize_snapshot();
    while(true) {
      auto message = OtcLinkMessage();
      auto sequence_number = std::uint32_t(0);
      try {
        message = m_otc_link_client->read(Beam::out(sequence_number));
      } catch(const Beam::EndOfFileException&) {
        break;
      }
      if(sequence_number <= last_sequence_number) {
        continue;
      }
      last_sequence_number = sequence_number;
      parse_message(message);
    }
  }
}

#endif
