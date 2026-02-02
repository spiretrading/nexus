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
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkClient.hpp"

namespace Nexus {

  /**
   * Parses packets from the OTC Link data feed.
   * @param <M> The type of MarketDataFeedClient used to update the
   *        MarketDataServer.
   * @param <O> The type of OtcLinkClient receiving messages.
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
       * @param feed_client Initializes the MarketDataFeedClient.
       * @param otc_link_client The OtcLinkClient receiving messages.
       */
      template<Beam::Initializes<M> MF, Beam::Initializes<O> OF>
      OtcLinkMarketDataFeedClient(MF&& feed_client, OF&& otc_link_client);

      ~OtcLinkMarketDataFeedClient();

      void close();

    private:
      struct StoredQuote {
        Security m_security;
        BookQuote m_bid;
        BookQuote m_ask;
      };
      Beam::local_ptr_t<M> m_feed_client;
      Beam::local_ptr_t<O> m_otc_link_client;
      std::unordered_map<std::uint32_t, StoredQuote> m_quotes;
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
      void read_loop();
  };

  template<typename M, typename O>
  template<Beam::Initializes<M> MF, Beam::Initializes<O> OF>
  OtcLinkMarketDataFeedClient<M, O>::OtcLinkMarketDataFeedClient(
      MF&& feed_client, OF&& otc_link_client)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_feed_client(std::forward<MF>(feed_client)),
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
      auto i = m_quotes.find(quote_id);
      if(i == m_quotes.end()) {
        return;
      }
      auto& stored = i->second;
      auto bid = stored.m_bid;
      bid.m_quote.m_size = 0;
      bid.m_timestamp = parse_timestamp(data + 52);
      std::cout << "OTC LINK DELETE: " <<
        SecurityBookQuote(bid, stored.m_security) << std::endl;
//      m_feed_client->publish(SecurityBookQuote(bid, stored.m_security));
      auto ask = stored.m_ask;
      ask.m_quote.m_size = 0;
      ask.m_timestamp = parse_timestamp(data + 31);
      std::cout << "OTC LINK DELETE: " <<
        SecurityBookQuote(ask, stored.m_security) << std::endl;
//      m_feed_client->publish(SecurityBookQuote(ask, stored.m_security));
      m_quotes.erase(i);
      return;
    }
    auto security_id = parse_uint32(data + 10);
    auto mpid = std::string(data + 14, 4);
    auto ask_price = parse_money(data + 18);
    auto ask_size = parse_quantity(data + 26);
    auto ask_timestamp = parse_timestamp(data + 31);
    auto bid_price = parse_money(data + 39);
    auto bid_size = parse_quantity(data + 47);
    auto bid_timestamp = parse_timestamp(data + 52);
    auto security = Security(std::to_string(security_id), Venue("OTCM"));
    auto bid = BookQuote(
      mpid, true, Venue("OTCM"), make_bid(bid_price, bid_size), bid_timestamp);
    auto ask = BookQuote(
      mpid, true, Venue("OTCM"), make_ask(ask_price, ask_size), ask_timestamp);
    m_quotes.insert(std::pair(quote_id, StoredQuote(security, bid, ask)));
    std::cout <<
      "OTC LINK ADD: " << SecurityBookQuote(bid, security) << std::endl;
//    m_feed_client->publish(SecurityBookQuote(bid, security));
    std::cout <<
      "OTC LINK ADD: " << SecurityBookQuote(ask, security) << std::endl;
//    m_feed_client->publish(SecurityBookQuote(ask, security));
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
      }
    }
  }
}

#endif
