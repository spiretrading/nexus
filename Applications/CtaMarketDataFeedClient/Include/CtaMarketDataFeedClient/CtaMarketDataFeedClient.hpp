#ifndef NEXUS_CTA_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_CTA_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "CtaMarketDataFeedClient/CtaConfiguration.hpp"
#include "CtaMarketDataFeedClient/CtaMessage.hpp"

namespace Nexus::MarketDataService {

  /**
   * Parses packets from a CTA market data feed.
   * @param <M> The type of MarketDataFeedClient used to update the
   *        MarketDataServer.
   * @param <P> The type of client receiving messages.
   */
  template<typename M, typename P>
  class CtaMarketDataFeedClient : private boost::noncopyable {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::GetTryDereferenceType<M>;

      /** The type of client receiving messages. */
      using ProtocolClient = Beam::GetTryDereferenceType<P>;

      /**
       * Constructs a CtaMarketDataFeedClient.
       * @param config The configuration to use.
       * @param marketDataFeedClient Initializes the MarketDataFeedClient.
       * @param protocolClient The client receiving messages.
       */
      template<typename MF, typename PF>
      CtaMarketDataFeedClient(CtaConfiguration config,
        MF&& marketDataFeedClient, PF&& protocolClient);

      ~CtaMarketDataFeedClient();

      void Close();

    private:
      CtaConfiguration m_config;
      Beam::GetOptionalLocalPtr<M>
        m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<P> m_protocolClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      char ParseChar(Beam::Out<const char*> cursor);
      std::string ParseAlphanumeric(std::size_t size,
        Beam::Out<const char*> cursor);
      std::string ParseSymbol(std::size_t size, Beam::Out<const char*> cursor);
      template<typename T>
      Quantity ParseNumeric(Beam::Out<const char*> cursor);
      Money ParseMoney(std::size_t length, Beam::Out<const char*> cursor);
      MarketCode ParseMarket(std::uint8_t identifier);
      MarketCode ParseMarket(Beam::Out<const char*> cursor);
      Quote HandleShortNationalBboAppendage(Side side,
        Beam::Out<const char*> cursor);
      Quote HandleLongNationalBboAppendage(Side side,
        Beam::Out<const char*> cursor);
      void HandleShortFormMarketQuoteMessage(const CtaMessage& message);
      void HandleLongFormMarketQuoteMessage(const CtaMessage& message);
      void HandleShortFormTradeMessage(const CtaMessage& message);
      void HandleLongFormTradeMessage(const CtaMessage& message);
      void Dispatch(const CtaMessage& message);
      void ReadLoop();
  };

  template<typename M, typename P>
  template<typename MF, typename PF>
  CtaMarketDataFeedClient<M, P>::CtaMarketDataFeedClient(
    CtaConfiguration config, MF&& marketDataFeedClient, PF&& protocolClient)
    : m_config(std::move(config)),
      m_marketDataFeedClient(std::forward<MF>(marketDataFeedClient)),
      m_protocolClient(std::forward<PF>(protocolClient)),
      m_readLoopRoutine(Beam::Routines::Spawn(
        std::bind(&CtaMarketDataFeedClient::ReadLoop, this))) {}

  template<typename M, typename P>
  CtaMarketDataFeedClient<M, P>::~CtaMarketDataFeedClient() {
    Close();
  }

  template<typename M, typename P>
  void CtaMarketDataFeedClient<M, P>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_protocolClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.Close();
  }

  template<typename M, typename P>
  char CtaMarketDataFeedClient<M, P>::ParseChar(Beam::Out<const char*> cursor) {
    auto value = **cursor;
    ++*cursor;
    return value;
  }

  template<typename M, typename P>
  std::string CtaMarketDataFeedClient<M, P>::ParseAlphanumeric(std::size_t size,
      Beam::Out<const char*> cursor) {
    auto value = std::string();
    auto token = *cursor;
    while(size > 0) {
      if(*token != ' ') {
        value += *token;
        ++token;
        --size;
      } else {
        token += size;
        size = 0;
      }
    }
    *cursor = token;
    return value;
  }

  template<typename M, typename P>
  std::string CtaMarketDataFeedClient<M, P>::ParseSymbol(std::size_t size,
      Beam::Out<const char*> cursor) {
    auto value = std::string();
    auto token = *cursor;
    auto state = 0;
    auto suffix = std::string();
    while(size > 0) {
      if(state == 0) {
        if(*token == '.' || *token == 'p' || *token == 'r' || *token == 'w') {
          state = 1;
        } else if(*token != ' ') {
          value += *token;
          ++token;
          --size;
        } else {
          token += size;
          size = 0;
        }
      }
      if(state == 1) {
        if(*token != ' ') {
          suffix += *token;
          ++token;
          --size;
        } else {
          token += size;
          size = 0;
        }
      }
    }
    *cursor = token;
    if(!suffix.empty()) {
      value += '.';
      auto suffixToken = suffix.c_str();
      auto suffixSize = suffix.size();
      while(suffixSize > 0) {
        if(*suffixToken == 'p') {
          value += "PR";
          ++suffixToken;
          --suffixSize;
        } else if(*suffixToken == 'r') {
          value += "RT";
          ++suffixToken;
          --suffixSize;
        } else if(*suffixToken == 'w') {
          value += "WI";
          ++suffixToken;
          --suffixSize;
        } else if(*suffixToken == '.') {
          ++suffixToken;
          --suffixSize;
        } else {
          value += *suffixToken;
          ++suffixToken;
          --suffixSize;
        }
      }
    }
    return value;
  }

  template<typename M, typename P>
  template<typename T>
  Quantity CtaMarketDataFeedClient<M, P>::ParseNumeric(
      Beam::Out<const char*> cursor) {
    auto token = *cursor;
    auto value = static_cast<Quantity>(Beam::FromBigEndian(
      *reinterpret_cast<const T*>(token)));
    token += sizeof(T);
    *cursor = token;
    return value;
  }

  template<typename M, typename P>
  Money CtaMarketDataFeedClient<M, P>::ParseMoney(std::size_t length,
      Beam::Out<const char*> cursor) {
    static constexpr auto SHORT_FORM = 2;
    static constexpr auto LONG_FORM = 8;
    auto rawValue = std::uint64_t();
    auto value = Money();
    auto token = *cursor;
    if(length == SHORT_FORM) {
      rawValue = Beam::FromBigEndian(
        *reinterpret_cast<const std::uint16_t*>(token));
      value = (rawValue * Money::ONE) / 100;
      token += sizeof(std::uint16_t);
    } else if(length == LONG_FORM) {
      rawValue = Beam::FromBigEndian(
        *reinterpret_cast<const std::uint64_t*>(token));
      value = (rawValue * Money::ONE) / 1000000;
      token += sizeof(std::uint64_t);
    } else {
      BOOST_THROW_EXCEPTION(std::runtime_error("Unknown price format."));
    }
    *cursor = token;
    return value;
  }

  template<typename M, typename P>
  MarketCode CtaMarketDataFeedClient<M, P>::ParseMarket(
      std::uint8_t identifier) {
    return m_config.m_marketCodes[identifier];
  }

  template<typename M, typename P>
  MarketCode CtaMarketDataFeedClient<M, P>::ParseMarket(
      Beam::Out<const char*> cursor) {
    auto value = static_cast<std::uint8_t>(**cursor);
    auto code = ParseMarket(value);
    ++*cursor;
    return code;
  }

  template<typename M, typename P>
  Quote CtaMarketDataFeedClient<M, P>::HandleShortNationalBboAppendage(
      Side side, Beam::Out<const char*> cursor) {
    static constexpr auto SHORT_FORM = 2;
    auto token = *cursor;
    token += sizeof(std::uint8_t);
    auto price = ParseMoney(SHORT_FORM, Beam::Store(token));
    auto size = ParseNumeric<std::uint16_t>(Beam::Store(token));
    *cursor = token;
    return Quote{price, size, side};
  }

  template<typename M, typename P>
  Quote CtaMarketDataFeedClient<M, P>::HandleLongNationalBboAppendage(Side side,
      Beam::Out<const char*> cursor) {
    static constexpr auto LONG_FORM = 8;
    auto token = *cursor;
    token += sizeof(std::uint8_t);
    token += sizeof(std::uint8_t);
    auto price = ParseMoney(LONG_FORM, Beam::Store(token));
    auto size = ParseNumeric<std::uint32_t>(Beam::Store(token));
    token += sizeof(std::uint32_t);
    *cursor = token;
    return Quote{price, size, side};
  }

  template<typename M, typename P>
  void CtaMarketDataFeedClient<M, P>::HandleShortFormMarketQuoteMessage(
      const CtaMessage& message) {
    constexpr auto SYMBOL_LENGTH = 5;
    constexpr auto PRICE_LENGTH = 2;
    constexpr auto LOT_SIZE = 100;
    auto cursor = message.m_body;
    auto symbol = ParseSymbol(SYMBOL_LENGTH, Beam::Store(cursor));
    auto bidPrice = ParseMoney(PRICE_LENGTH, Beam::Store(cursor));
    auto bidSize = LOT_SIZE * ParseNumeric<std::uint16_t>(Beam::Store(cursor));
    auto askPrice = ParseMoney(PRICE_LENGTH, Beam::Store(cursor));
    auto askSize = LOT_SIZE * ParseNumeric<std::uint16_t>(Beam::Store(cursor));
    auto primaryMarket = ParseMarket(Beam::Store(cursor));
    auto nationalBboIndicator = ParseChar(Beam::Store(cursor));
    auto market = ParseMarket(message.m_header.m_participantId);
    auto security = Security(symbol, primaryMarket, m_config.m_country);
    auto bid = Quote(bidPrice, bidSize, Side::BID);
    auto ask = Quote(askPrice, askSize, Side::ASK);
    if(nationalBboIndicator == 'G') {
      auto bboQuote = BboQuote(bid, ask, message.m_header.m_timestamp);
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(nationalBboIndicator == 'T') {
      auto bboBid = HandleShortNationalBboAppendage(Side::BID,
        Beam::Store(cursor));
      auto bboAsk = HandleShortNationalBboAppendage(Side::ASK,
        Beam::Store(cursor));
      auto bboQuote = BboQuote(bboBid, bboAsk, message.m_header.m_timestamp);
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(nationalBboIndicator == 'U') {
      auto bboBid = HandleLongNationalBboAppendage(Side::BID,
        Beam::Store(cursor));
      auto bboAsk = HandleLongNationalBboAppendage(Side::ASK,
        Beam::Store(cursor));
      auto bboQuote = BboQuote(bboBid, bboAsk, message.m_header.m_timestamp);
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    }
    auto marketQuote = MarketQuote(market, bid, ask,
      message.m_header.m_timestamp);
    m_marketDataFeedClient->PublishMarketQuote(
      SecurityMarketQuote{marketQuote, security});
  }

  template<typename M, typename P>
  void CtaMarketDataFeedClient<M, P>::HandleLongFormMarketQuoteMessage(
      const CtaMessage& message) {
    constexpr auto SYMBOL_LENGTH = 11;
    constexpr auto PRICE_LENGTH = 8;
    constexpr auto LOT_SIZE = 100;
    auto cursor = message.m_body;
    auto symbol = ParseSymbol(SYMBOL_LENGTH, Beam::Store(cursor));
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint8_t);
    auto bidPrice = ParseMoney(PRICE_LENGTH, Beam::Store(cursor));
    auto bidSize = LOT_SIZE * ParseNumeric<std::uint32_t>(Beam::Store(cursor));
    auto askPrice = ParseMoney(PRICE_LENGTH, Beam::Store(cursor));
    auto askSize = LOT_SIZE * ParseNumeric<std::uint32_t>(Beam::Store(cursor));
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint32_t);
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint64_t);
    cursor += sizeof(std::uint8_t);
    auto primaryMarket = ParseMarket(Beam::Store(cursor));
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint8_t);
    auto nationalBboIndicator = ParseChar(Beam::Store(cursor));
    auto market = ParseMarket(message.m_header.m_participantId);
    auto security = Security(symbol, primaryMarket, m_config.m_country);
    auto bid = Quote(bidPrice, bidSize, Side::BID);
    auto ask = Quote(askPrice, askSize, Side::ASK);
    if(nationalBboIndicator == 'G') {
      auto bboQuote = BboQuote(bid, ask, message.m_header.m_timestamp);
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(nationalBboIndicator == 'T') {
      auto bboBid = HandleShortNationalBboAppendage(Side::BID,
        Beam::Store(cursor));
      auto bboAsk = HandleShortNationalBboAppendage(Side::ASK,
        Beam::Store(cursor));
      auto bboQuote = BboQuote(bboBid, bboAsk, message.m_header.m_timestamp);
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(nationalBboIndicator == 'U') {
      auto bboBid = HandleLongNationalBboAppendage(Side::BID,
        Beam::Store(cursor));
      auto bboAsk = HandleLongNationalBboAppendage(Side::ASK,
        Beam::Store(cursor));
      auto bboQuote = BboQuote(bboBid, bboAsk, message.m_header.m_timestamp);
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    }
    auto marketQuote = MarketQuote(market, bid, ask,
      message.m_header.m_timestamp);
    m_marketDataFeedClient->PublishMarketQuote(
      SecurityMarketQuote{marketQuote, security});
  }

  template<typename M, typename P>
  void CtaMarketDataFeedClient<M, P>::HandleShortFormTradeMessage(
      const CtaMessage& message) {
    constexpr auto SYMBOL_LENGTH = 5;
    constexpr auto PRICE_LENGTH = 2;
    auto cursor = message.m_body;
    auto symbol = ParseSymbol(SYMBOL_LENGTH, Beam::Store(cursor));
    auto saleCondition = std::string(1, ParseChar(Beam::Store(cursor)));
    cursor += sizeof(std::uint8_t);
    auto price = ParseMoney(PRICE_LENGTH, Beam::Store(cursor));
    auto quantity = ParseNumeric<std::uint16_t>(Beam::Store(cursor));
    auto primaryMarket = ParseMarket(Beam::Store(cursor));
    auto market = ParseMarket(message.m_header.m_participantId);
    auto condition = TimeAndSale::Condition(
      TimeAndSale::Condition::Type::REGULAR, saleCondition);
    auto timeAndSale = TimeAndSale(message.m_header.m_timestamp, price,
      quantity, condition, market.GetData());
    auto security = Security(symbol, primaryMarket, m_config.m_country);
    m_marketDataFeedClient->PublishTimeAndSale(
      SecurityTimeAndSale{timeAndSale, security});
  }

  template<typename M, typename P>
  void CtaMarketDataFeedClient<M, P>::HandleLongFormTradeMessage(
      const CtaMessage& message) {
    constexpr auto SYMBOL_LENGTH = 11;
    constexpr auto PRICE_LENGTH = 8;
    constexpr auto CONDITION_LENGTH = 4;
    auto cursor = message.m_body;
    auto symbol = ParseSymbol(SYMBOL_LENGTH, Beam::Store(cursor));
    cursor += sizeof(std::uint8_t);
    auto saleCondition = ParseAlphanumeric(CONDITION_LENGTH,
      Beam::Store(cursor));
    auto price = ParseMoney(PRICE_LENGTH, Beam::Store(cursor));
    auto quantity = ParseNumeric<std::uint32_t>(Beam::Store(cursor));
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint8_t);
    cursor += sizeof(std::uint64_t);
    cursor += sizeof(std::uint8_t);
    auto primaryMarket = ParseMarket(Beam::Store(cursor));
    auto market = ParseMarket(message.m_header.m_participantId);
    auto condition = TimeAndSale::Condition(
      TimeAndSale::Condition::Type::REGULAR, saleCondition);
    auto timeAndSale = TimeAndSale(message.m_header.m_timestamp, price,
      quantity, condition, market.GetData());
    auto security = Security(symbol, primaryMarket, m_config.m_country);
    m_marketDataFeedClient->PublishTimeAndSale(
      SecurityTimeAndSale{timeAndSale, security});
  }

  template<typename M, typename P>
  void CtaMarketDataFeedClient<M, P>::Dispatch(const CtaMessage& message) {
    if(message.m_header.m_category == 'Q') {
      if(message.m_header.m_type == 'L') {
        HandleLongFormMarketQuoteMessage(message);
      } else if(message.m_header.m_type == 'Q') {
        HandleShortFormMarketQuoteMessage(message);
      }
    } else if(message.m_header.m_category == 'T') {
      if(message.m_header.m_type == 'L') {
        HandleLongFormTradeMessage(message);
      } else if(message.m_header.m_type == 'T') {
        HandleShortFormTradeMessage(message);
      }
    }
  }

  template<typename M, typename P>
  void CtaMarketDataFeedClient<M, P>::ReadLoop() {
    while(true) {
      try {
        auto message = m_protocolClient->Read();
        if(m_config.m_isLoggingMessages) {
          std::cout << message.m_header.m_timestamp << ": " <<
            message.m_header.m_category << " " <<
            message.m_header.m_type << std::endl;
        }
        Dispatch(message);
      } catch(const Beam::IO::EndOfFileException&) {
        break;
      }
    }
  }
}

#endif
