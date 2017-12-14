#ifndef NEXUS_CTAMARKETDATAFEEDCLIENT_HPP
#define NEXUS_CTAMARKETDATAFEEDCLIENT_HPP
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

namespace Nexus {
namespace MarketDataService {

  /*! \class CtaMarketDataFeedClient
      \brief Parses packets from a CTA market data feed.
      \tparam MarketDataFeedClientType The type of MarketDataFeedClient used to
              update the MarketDataServer.
      \tparam ProtocolClientType The type of client receiving messages.
   */
  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  class CtaMarketDataFeedClient : private boost::noncopyable {
    public:

      //! The type of MarketDataFeedClient used to update the MarketDataServer.
      using MarketDataFeedClient =
        Beam::GetTryDereferenceType<MarketDataFeedClientType>;

      //! The type of client receiving messages.
      using ProtocolClient = Beam::GetTryDereferenceType<ProtocolClientType>;

      //! Constructs a CtaMarketDataFeedClient.
      /*!
        \param config The configuration to use.
        \param marketDataFeedClient Initializes the MarketDataFeedClient.
        \param protocolClient The client receiving messages.
      */
      template<typename MarketDataFeedClientForward,
        typename ProtocolClientForward>
      CtaMarketDataFeedClient(const CtaConfiguration& config,
        MarketDataFeedClientForward&& marketDataFeedClient,
        ProtocolClientForward&& protocolClient);

      ~CtaMarketDataFeedClient();

      void Open();

      void Close();

    private:
      CtaConfiguration m_config;
      Beam::GetOptionalLocalPtr<MarketDataFeedClientType>
        m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<ProtocolClientType> m_protocolClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      void Shutdown();
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

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  template<typename MarketDataFeedClientForward, typename ProtocolClientForward>
  CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      CtaMarketDataFeedClient(const CtaConfiguration& config,
      MarketDataFeedClientForward&& marketDataFeedClient,
      ProtocolClientForward&& protocolClient)
      : m_config{config},
        m_marketDataFeedClient{std::forward<MarketDataFeedClientForward>(
          marketDataFeedClient)},
        m_protocolClient{std::forward<ProtocolClientForward>(protocolClient)} {}

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ~CtaMarketDataFeedClient() {
    Close();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataFeedClient->Open();
      m_protocolClient->Open();
      m_readLoopRoutine = Beam::Routines::Spawn(
        std::bind(&CtaMarketDataFeedClient::ReadLoop, this));
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Shutdown() {
    m_protocolClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.SetClosed();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  char CtaMarketDataFeedClient<MarketDataFeedClientType,
      ProtocolClientType>::ParseChar(Beam::Out<const char*> cursor) {
    auto value = **cursor;
    ++*cursor;
    return value;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  std::string CtaMarketDataFeedClient<MarketDataFeedClientType,
      ProtocolClientType>::ParseAlphanumeric(std::size_t size,
      Beam::Out<const char*> cursor) {
    std::string value;
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

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  std::string CtaMarketDataFeedClient<MarketDataFeedClientType,
      ProtocolClientType>::ParseSymbol(std::size_t size,
      Beam::Out<const char*> cursor) {
    std::string value;
    auto token = *cursor;
    auto state = 0;
    std::string suffix;
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

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  template<typename T>
  Quantity CtaMarketDataFeedClient<MarketDataFeedClientType,
      ProtocolClientType>::ParseNumeric(Beam::Out<const char*> cursor) {
    auto token = *cursor;
    auto value = static_cast<Quantity>(Beam::FromBigEndian(
      *reinterpret_cast<const T*>(token)));
    token += sizeof(T);
    *cursor = token;
    return value;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  Money CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ParseMoney(std::size_t length, Beam::Out<const char*> cursor) {
    static auto SHORT_FORM = 2;
    static auto LONG_FORM = 8;
    std::uint64_t rawValue;
    Money value;
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
      BOOST_THROW_EXCEPTION(std::runtime_error{"Unknown price format."});
    }
    *cursor = token;
    return value;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  MarketCode CtaMarketDataFeedClient<MarketDataFeedClientType,
      ProtocolClientType>::ParseMarket(std::uint8_t identifier) {
    return m_config.m_marketCodes[identifier];
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  MarketCode CtaMarketDataFeedClient<MarketDataFeedClientType,
      ProtocolClientType>::ParseMarket(Beam::Out<const char*> cursor) {
    auto value = static_cast<std::uint8_t>(**cursor);
    auto code = ParseMarket(value);
    ++*cursor;
    return code;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  Quote CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleShortNationalBboAppendage(Side side,
      Beam::Out<const char*> cursor) {
    static const auto SHORT_FORM = 2;
    auto token = *cursor;
    token += sizeof(std::uint8_t);
    auto price = ParseMoney(SHORT_FORM, Beam::Store(token));
    auto size = ParseNumeric<std::uint16_t>(Beam::Store(token));
    *cursor = token;
    return Quote{price, size, side};
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  Quote CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleLongNationalBboAppendage(Side side, Beam::Out<const char*> cursor) {
    static const auto LONG_FORM = 8;
    auto token = *cursor;
    token += sizeof(std::uint8_t);
    token += sizeof(std::uint8_t);
    auto price = ParseMoney(LONG_FORM, Beam::Store(token));
    auto size = ParseNumeric<std::uint32_t>(Beam::Store(token));
    token += sizeof(std::uint32_t);
    *cursor = token;
    return Quote{price, size, side};
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleShortFormMarketQuoteMessage(const CtaMessage& message) {
    const auto SYMBOL_LENGTH = 5;
    const auto PRICE_LENGTH = 2;
    const auto LOT_SIZE = 100;
    auto cursor = message.m_body;
    auto symbol = ParseSymbol(SYMBOL_LENGTH, Beam::Store(cursor));
    auto bidPrice = ParseMoney(PRICE_LENGTH, Beam::Store(cursor));
    auto bidSize = LOT_SIZE * ParseNumeric<std::uint16_t>(Beam::Store(cursor));
    auto askPrice = ParseMoney(PRICE_LENGTH, Beam::Store(cursor));
    auto askSize = LOT_SIZE * ParseNumeric<std::uint16_t>(Beam::Store(cursor));
    auto primaryMarket = ParseMarket(Beam::Store(cursor));
    auto nationalBboIndicator = ParseChar(Beam::Store(cursor));
    auto market = ParseMarket(message.m_header.m_participantId);
    Security security{symbol, primaryMarket, m_config.m_country};
    Quote bid{bidPrice, bidSize, Side::BID};
    Quote ask{askPrice, askSize, Side::ASK};
    if(nationalBboIndicator == 'G') {
      BboQuote bboQuote{bid, ask, message.m_header.m_timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(nationalBboIndicator == 'T') {
      auto bboBid = HandleShortNationalBboAppendage(Side::BID,
        Beam::Store(cursor));
      auto bboAsk = HandleShortNationalBboAppendage(Side::ASK,
        Beam::Store(cursor));
      BboQuote bboQuote{bboBid, bboAsk, message.m_header.m_timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(nationalBboIndicator == 'U') {
      auto bboBid = HandleLongNationalBboAppendage(Side::BID,
        Beam::Store(cursor));
      auto bboAsk = HandleLongNationalBboAppendage(Side::ASK,
        Beam::Store(cursor));
      BboQuote bboQuote{bboBid, bboAsk, message.m_header.m_timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    }
    MarketQuote marketQuote{market, bid, ask, message.m_header.m_timestamp};
    m_marketDataFeedClient->PublishMarketQuote(
      SecurityMarketQuote{marketQuote, security});
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleLongFormMarketQuoteMessage(const CtaMessage& message) {
    const auto SYMBOL_LENGTH = 11;
    const auto PRICE_LENGTH = 8;
    const auto LOT_SIZE = 100;
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
    Security security{symbol, primaryMarket, m_config.m_country};
    Quote bid{bidPrice, bidSize, Side::BID};
    Quote ask{askPrice, askSize, Side::ASK};
    if(nationalBboIndicator == 'G') {
      BboQuote bboQuote{bid, ask, message.m_header.m_timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(nationalBboIndicator == 'T') {
      auto bboBid = HandleShortNationalBboAppendage(Side::BID,
        Beam::Store(cursor));
      auto bboAsk = HandleShortNationalBboAppendage(Side::ASK,
        Beam::Store(cursor));
      BboQuote bboQuote{bboBid, bboAsk, message.m_header.m_timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(nationalBboIndicator == 'U') {
      auto bboBid = HandleLongNationalBboAppendage(Side::BID,
        Beam::Store(cursor));
      auto bboAsk = HandleLongNationalBboAppendage(Side::ASK,
        Beam::Store(cursor));
      BboQuote bboQuote{bboBid, bboAsk, message.m_header.m_timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    }
    MarketQuote marketQuote{market, bid, ask, message.m_header.m_timestamp};
    m_marketDataFeedClient->PublishMarketQuote(
      SecurityMarketQuote{marketQuote, security});
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleShortFormTradeMessage(const CtaMessage& message) {
    const auto SYMBOL_LENGTH = 5;
    const auto PRICE_LENGTH = 2;
    auto cursor = message.m_body;
    auto symbol = ParseSymbol(SYMBOL_LENGTH, Beam::Store(cursor));
    auto saleCondition = ParseChar(Beam::Store(cursor));
    cursor += sizeof(std::uint8_t);
    auto price = ParseMoney(PRICE_LENGTH, Beam::Store(cursor));
    auto quantity = ParseNumeric<std::uint16_t>(Beam::Store(cursor));
    auto primaryMarket = ParseMarket(Beam::Store(cursor));
    auto market = ParseMarket(message.m_header.m_participantId);
    TimeAndSale::Condition condition{
      TimeAndSale::Condition::Type::REGULAR, std::string{saleCondition}};
    TimeAndSale timeAndSale{message.m_header.m_timestamp, price, quantity,
      condition, market.GetData()};
    Security security{symbol, primaryMarket, m_config.m_country};
    m_marketDataFeedClient->PublishTimeAndSale(
      SecurityTimeAndSale{timeAndSale, security});
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleLongFormTradeMessage(const CtaMessage& message) {
    const auto SYMBOL_LENGTH = 11;
    const auto PRICE_LENGTH = 8;
    const auto CONDITION_LENGTH = 4;
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
    TimeAndSale::Condition condition{
      TimeAndSale::Condition::Type::REGULAR, saleCondition};
    TimeAndSale timeAndSale{message.m_header.m_timestamp, price, quantity,
      condition, market.GetData()};
    Security security{symbol, primaryMarket, m_config.m_country};
    m_marketDataFeedClient->PublishTimeAndSale(
      SecurityTimeAndSale{timeAndSale, security});
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Dispatch(const CtaMessage& message) {
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

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ReadLoop() {
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
}

#endif
