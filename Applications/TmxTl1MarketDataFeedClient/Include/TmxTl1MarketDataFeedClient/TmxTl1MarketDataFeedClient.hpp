#ifndef NEXUS_TMX_TL1_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_TMX_TL1_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/Stamp/StampMessage.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1Configuration.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1ServiceAccessClient.hpp"

namespace Nexus {

  /**
   * Parses packets from the TMX TL1 feed.
   * @param <M> The type of MarketDataFeedClient used to update the
   *        MarketDataServer.
   * @param S The type of service access client receiving messages.
   */
  template<typename M, typename S>
  class TmxTl1MarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::dereference_t<M>;

      /** The type of channel receiving the market data feed. */
      using ServiceAccessClient = Beam::dereference_t<S>;

      /**
       * Constructs a TmxTl1MarketDataFeedClient.
       * @param config The configuration to use.
       * @param feed_client Initializes the MarketDataFeedClient.
       * @param service_access_client The service access client receiving
       *        messages.
       */
      template<typename MF, typename SF>
      TmxTl1MarketDataFeedClient(TmxTl1Configuration config, MF&& feed_client,
        SF&& service_access_client);

      ~TmxTl1MarketDataFeedClient();

      void close();

    private:
      TmxTl1Configuration m_config;
      Beam::local_ptr_t<M> m_feed_client;
      Beam::local_ptr_t<S> m_service_access_client;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      TmxTl1MarketDataFeedClient(const TmxTl1MarketDataFeedClient&) = delete;
      TmxTl1MarketDataFeedClient& operator =(
        const TmxTl1MarketDataFeedClient&) = delete;
      boost::optional<Money> parse_money(
        const char* token, int integral_size, int fractional_size);
      boost::optional<int> parse_quantity(const char* token, int size);
      boost::optional<boost::posix_time::ptime> parse_timestamp(
        const char* token);
      void handle_equity_quote_message(const StampPacket& message);
      void read_loop();
  };

  template<typename M, typename S>
  template<typename MF, typename SF>
  TmxTl1MarketDataFeedClient<M, S>::TmxTl1MarketDataFeedClient(
      TmxTl1Configuration config, MF&& feed_client, SF&& service_access_client)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_config(std::move(config)),
            m_feed_client(std::forward<MF>(feed_client)),
            m_service_access_client(std::forward<SF>(service_access_client)),
            m_read_loop(Beam::spawn(
              std::bind(&TmxTl1MarketDataFeedClient::read_loop, this))) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Failed to initialize the TMX TL1 market data feed client."));
  }

  template<typename M, typename S>
  TmxTl1MarketDataFeedClient<M, S>::~TmxTl1MarketDataFeedClient() {
    close();
  }

  template<typename M, typename S>
  void TmxTl1MarketDataFeedClient<M, S>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_service_access_client->close();
    m_feed_client->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename M, typename S>
  boost::optional<boost::posix_time::ptime>
      TmxTl1MarketDataFeedClient<M, S>::parse_timestamp(const char* token) {
    auto value = std::string(token, 23);
    auto y = boost::lexical_cast<int>(value.substr(0, 4));
    auto m = boost::lexical_cast<int>(value.substr(4, 2));
    auto d = boost::lexical_cast<int>(value.substr(6, 2));
    auto hr = boost::lexical_cast<int>(value.substr(8, 2));
    auto mn = boost::lexical_cast<int>(value.substr(10, 2));
    auto sec = boost::lexical_cast<int>(value.substr(12, 2));
    auto ns = boost::lexical_cast<int>(value.substr(14, 9));
    auto timestamp = boost::posix_time::ptime(
      boost::gregorian::date(static_cast<unsigned short>(y),
      static_cast<unsigned short>(m), static_cast<unsigned short>(d)),
      boost::posix_time::hours(hr) + boost::posix_time::minutes(mn) +
      boost::posix_time::seconds(sec) +
      boost::posix_time::microseconds(ns / 1000));
    return timestamp + m_config.m_time_offset;
  }

  template<typename M, typename S>
  boost::optional<Money> TmxTl1MarketDataFeedClient<M, S>::parse_money(
      const char* token, int integral_size, int fractional_size) {
    auto quantity = parse_quantity(token, integral_size + fractional_size);
    if(!quantity) {
      return boost::none;
    }
    auto value = *quantity * Money::ONE;
    for(auto i = 0; i < fractional_size; ++i) {
      value /= 10;
    }
    return value;
  }

  template<typename M, typename S>
  boost::optional<int> TmxTl1MarketDataFeedClient<M, S>::parse_quantity(
      const char* token, int size) {
    auto quantity = 0;
    for(auto i = 0; i < size; ++i) {
      if(!std::isdigit(*token)) {
        return boost::none;
      }
      quantity = quantity * 10 + (*token - '0');
      ++token;
    }
    return quantity;
  }

  template<typename M, typename S>
  void TmxTl1MarketDataFeedClient<M, S>::handle_equity_quote_message(
      const StampPacket& message) {
    constexpr auto SYMBOL_SIZE = 12;
    constexpr auto PRICE_SIZE = 9;
    constexpr auto PRICE_INTEGRAL_SIZE = 6;
    constexpr auto PRICE_FRACTIONAL_SIZE = 3;
    constexpr auto VOLUME_SIZE = 9;
    constexpr auto TIMESTAMP_SIZE = 23;
    auto remaining_size = message.m_message_size;
    auto token = message.m_message;
    if(remaining_size < SYMBOL_SIZE) {
      return;
    }
    auto symbol = std::string();
    auto symbol_token = token;
    while(symbol.size() < SYMBOL_SIZE && !std::isspace(*symbol_token)) {
      symbol += *symbol_token;
      ++symbol_token;
    }
    token += SYMBOL_SIZE;
    remaining_size -= SYMBOL_SIZE;
    if(remaining_size < PRICE_SIZE) {
      return;
    }
    auto bid_price =
      parse_money(token, PRICE_INTEGRAL_SIZE, PRICE_FRACTIONAL_SIZE);
    if(!bid_price) {
      return;
    }
    token += PRICE_SIZE;
    remaining_size -= PRICE_SIZE;
    if(remaining_size < VOLUME_SIZE) {
      return;
    }
    auto bid_volume = parse_quantity(token, VOLUME_SIZE);
    if(!bid_volume) {
      return;
    }
    token += VOLUME_SIZE;
    remaining_size -= VOLUME_SIZE;
    if(remaining_size < PRICE_SIZE) {
      return;
    }
    auto ask_price =
      parse_money(token, PRICE_INTEGRAL_SIZE, PRICE_FRACTIONAL_SIZE);
    if(!ask_price) {
      return;
    }
    token += PRICE_SIZE;
    remaining_size -= PRICE_SIZE;
    auto ask_volume = parse_quantity(token, VOLUME_SIZE);
    if(!ask_volume) {
      return;
    }
    token += VOLUME_SIZE;
    remaining_size -= VOLUME_SIZE;
    if(remaining_size < TIMESTAMP_SIZE) {
      return;
    }
    auto timestamp = parse_timestamp(token);
    if(!timestamp) {
      return;
    }
    auto security = Security(std::move(symbol), m_config.m_venue);
    auto bid = make_bid(*bid_price, *bid_volume);
    auto ask = make_ask(*ask_price, *ask_volume);
    auto bbo = BboQuote(bid, ask, *timestamp);
    m_feed_client->publish(SecurityBboQuote(bbo, security));
  }

  template<typename M, typename S>
  void TmxTl1MarketDataFeedClient<M, S>::read_loop() {
    while(true) {
      auto message = std::optional<StampPacket>();
      try {
        message.emplace(m_service_access_client->read());
      } catch(const Beam::EndOfFileException&) {
        break;
      }
      if(m_config.m_is_logging_messages) {
        std::cout << message->m_header.m_sequence_number << ": " <<
          message->m_header.m_message_type << " " <<
          std::string(message->m_message, message->m_message_size) << "\n";
      }
      if(message->m_header.m_message_type == "E ") {
        handle_equity_quote_message(*message);
      }
    }
  }
}

#endif
