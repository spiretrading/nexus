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
      Quantity ParseNumeric(std::size_t length, Beam::Out<const char*> cursor);
      Money ParseMoney(std::size_t length, char denominatorType,
        Beam::Out<const char*> cursor);
      MarketCode ParseMarket(std::uint8_t identifier);
      boost::posix_time::ptime ParseTimestamp(const std::array<char, 6>& code);
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
      : m_config(config),
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
  Quantity CtaMarketDataFeedClient<MarketDataFeedClientType,
      ProtocolClientType>::ParseNumeric(std::size_t length,
      Beam::Out<const char*> cursor) {
    Quantity value = 0;
    auto token = *cursor;
    for(std::size_t i = 0; i < length; ++i) {
      value = 10 * value + (*token - '0');
      ++token;
    }
    *cursor = token;
    return value;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  Money CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ParseMoney(std::size_t length, char denominatorType,
      Beam::Out<const char*> cursor) {
    if(denominatorType < 'A' || denominatorType > 'I') {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Invalid price."});
    }
    auto decimalDigits = static_cast<std::uint8_t>(
      (denominatorType - 'A') + 1);
    auto dollarValue = 0;
    auto dollarDigits = length - decimalDigits;
    auto token = *cursor;
    for(std::uint8_t i = 0; i < dollarDigits; ++i) {
      dollarValue = 10 * dollarValue + (*token - '0');
      ++token;
    }
    auto decimalValue = 0;
    for(std::uint8_t i = 0; i < decimalDigits; ++i) {
      decimalValue = 10 * decimalValue + (*token - '0');
      ++token;
    }
    *cursor = token;
    return dollarValue * Money::ONE +
      Beam::PowerOfTen(Money::DECIMAL_PLACES - decimalDigits) *
      decimalValue * Money::EPSILON;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  MarketCode CtaMarketDataFeedClient<MarketDataFeedClientType,
      ProtocolClientType>::ParseMarket(std::uint8_t identifier) {
    return m_config.m_marketCodes[identifier];
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  boost::posix_time::ptime CtaMarketDataFeedClient<MarketDataFeedClientType,
      ProtocolClientType>::ParseTimestamp(const std::array<char, 6>& code) {
    const auto BASE = 95;
    std::uint64_t microseconds = 0;
    for(auto digit : code) {
      microseconds = BASE * microseconds + (digit - ' ');
    }
    return m_config.m_timeOrigin +
      boost::posix_time::microseconds(microseconds);
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleShortFormMarketQuoteMessage(const CtaMessage& message) {
    const auto SYMBOL_LENGTH = 3;
    const auto PRICE_DIGITS = 8;
    const auto LOT_SIZE = 100;
    const auto LOT_DIGITS = 3;
    auto cursor = message.m_data;
    auto symbol = ParseAlphanumeric(SYMBOL_LENGTH, Beam::Store(cursor));
    ++cursor;
    ++cursor;
    ++cursor;
    auto timestamp = ParseTimestamp(message.m_cqsTimestamp);
    auto bidDenominatorIndicator = ParseChar(Beam::Store(cursor));
    auto bidPrice = ParseMoney(PRICE_DIGITS, bidDenominatorIndicator,
      Beam::Store(cursor));
    auto bidSize = LOT_SIZE * ParseNumeric(LOT_DIGITS, Beam::Store(cursor));
    ++cursor;
    auto askDenominatorIndicator = ParseChar(Beam::Store(cursor));
    auto askPrice = ParseMoney(PRICE_DIGITS, askDenominatorIndicator,
      Beam::Store(cursor));
    auto askSize = LOT_SIZE * ParseNumeric(LOT_DIGITS, Beam::Store(cursor));
    ++cursor;
    auto nationalBboIndicator = ParseChar(Beam::Store(cursor));
    auto finraBboIndicator = ParseChar(Beam::Store(cursor));
    auto market = ParseMarket(message.m_participantId);
    Security security{symbol, m_config.m_market, m_config.m_country};
    Quote bid{bidPrice, bidSize, Side::BID};
    Quote ask{askPrice, askSize, Side::ASK};
    if(nationalBboIndicator == '1') {
      BboQuote bboQuote{bid, ask, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(nationalBboIndicator == '4') {
      const auto LONG_FORM_PRICE_DIGITS = 12;
      const auto LONG_FORM_LOT_DIGITS = 7;
      cursor += 2;
      ++cursor;
      auto bboBidDenominatorIndicator = ParseChar(Beam::Store(cursor));
      auto bboBidPrice = ParseMoney(LONG_FORM_PRICE_DIGITS,
        bboBidDenominatorIndicator, Beam::Store(cursor));
      auto bboBidSize = LOT_SIZE * ParseNumeric(LONG_FORM_LOT_DIGITS,
        Beam::Store(cursor));
      ++cursor;
      ++cursor;
      auto bboAskDenominatorIndicator = ParseChar(Beam::Store(cursor));
      auto bboAskPrice = ParseMoney(LONG_FORM_PRICE_DIGITS,
        bboAskDenominatorIndicator, Beam::Store(cursor));
      auto bboAskSize = LOT_SIZE * ParseNumeric(LONG_FORM_LOT_DIGITS,
        Beam::Store(cursor));
      BboQuote bboQuote{Quote{bboBidPrice, bboBidSize, Side::BID},
        Quote{bboAskPrice, bboAskSize, Side::ASK}, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(nationalBboIndicator == '6') {
      ++cursor;
      auto bboBidDenominatorIndicator = ParseChar(Beam::Store(cursor));
      auto bboBidPrice = ParseMoney(PRICE_DIGITS, bboBidDenominatorIndicator,
        Beam::Store(cursor));
      auto bboBidSize = LOT_SIZE * ParseNumeric(LOT_DIGITS,
        Beam::Store(cursor));
      ++cursor;
      ++cursor;
      auto bboAskDenominatorIndicator = ParseChar(Beam::Store(cursor));
      auto bboAskPrice = ParseMoney(PRICE_DIGITS, bboAskDenominatorIndicator,
        Beam::Store(cursor));
      auto bboAskSize = LOT_SIZE * ParseNumeric(LOT_DIGITS,
        Beam::Store(cursor));
      BboQuote bboQuote{Quote{bboBidPrice, bboBidSize, Side::BID},
        Quote{bboAskPrice, bboAskSize, Side::ASK}, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    }
    MarketQuote marketQuote{market, bid, ask, timestamp};
    m_marketDataFeedClient->PublishMarketQuote(
      SecurityMarketQuote{marketQuote, security});
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleLongFormMarketQuoteMessage(const CtaMessage& message) {
    const auto SYMBOL_LENGTH = 11;
    const auto PRICE_DIGITS = 12;
    const auto LOT_DIGITS = 7;
    const auto LOT_SIZE = 100;
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(message.m_cqsTimestamp);
    auto market = ParseMarket(message.m_participantId);
    auto symbol = ParseAlphanumeric(SYMBOL_LENGTH, Beam::Store(cursor));
    ++cursor;
    auto testMessageIndicator = ParseChar(Beam::Store(cursor));
    if(testMessageIndicator == 'T') {
      return;
    }
    ++cursor;
    ++cursor;
    ++cursor;
    ++cursor;
    cursor += 3;
    ++cursor;
    ++cursor;
    ++cursor;
    ++cursor;
    ++cursor;
    ++cursor;
    ++cursor;
    auto bidDenominatorIndicator = ParseChar(Beam::Store(cursor));
    auto bidPrice = ParseMoney(PRICE_DIGITS, bidDenominatorIndicator,
      Beam::Store(cursor));
    auto bidSize = LOT_SIZE * ParseNumeric(LOT_DIGITS, Beam::Store(cursor));
    auto askDenominatorIndicator = ParseChar(Beam::Store(cursor));
    auto askPrice = ParseMoney(PRICE_DIGITS, askDenominatorIndicator,
      Beam::Store(cursor));
    auto askSize = LOT_SIZE * ParseNumeric(LOT_DIGITS, Beam::Store(cursor));
    cursor += 4;
    ++cursor;
    ++cursor;
    ++cursor;
    ++cursor;
    ++cursor;
    auto nationalBboIndicator = ParseChar(Beam::Store(cursor));
    auto finraBboIndicator = ParseChar(Beam::Store(cursor));
    Security security{symbol, m_config.m_market, m_config.m_country};
    Quote bid{bidPrice, bidSize, Side::BID};
    Quote ask{askPrice, askSize, Side::ASK};
    if(nationalBboIndicator == '1') {
      BboQuote bboQuote{bid, ask, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(nationalBboIndicator == '4') {
      const auto LONG_FORM_PRICE_DIGITS = 12;
      const auto LONG_FORM_LOT_DIGITS = 7;
      cursor += 2;
      ++cursor;
      auto bboBidDenominatorIndicator = ParseChar(Beam::Store(cursor));
      auto bboBidPrice = ParseMoney(LONG_FORM_PRICE_DIGITS,
        bboBidDenominatorIndicator, Beam::Store(cursor));
      auto bboBidSize = LOT_SIZE * ParseNumeric(LONG_FORM_LOT_DIGITS,
        Beam::Store(cursor));
      ++cursor;
      ++cursor;
      auto bboAskDenominatorIndicator = ParseChar(Beam::Store(cursor));
      auto bboAskPrice = ParseMoney(LONG_FORM_PRICE_DIGITS,
        bboAskDenominatorIndicator, Beam::Store(cursor));
      auto bboAskSize = LOT_SIZE * ParseNumeric(LONG_FORM_LOT_DIGITS,
        Beam::Store(cursor));
      BboQuote bboQuote{Quote{bboBidPrice, bboBidSize, Side::BID},
        Quote{bboAskPrice, bboAskSize, Side::ASK}, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(nationalBboIndicator == '6') {
      ++cursor;
      auto bboBidDenominatorIndicator = ParseChar(Beam::Store(cursor));
      auto bboBidPrice = ParseMoney(PRICE_DIGITS, bboBidDenominatorIndicator,
        Beam::Store(cursor));
      auto bboBidSize = LOT_SIZE * ParseNumeric(LOT_DIGITS,
        Beam::Store(cursor));
      ++cursor;
      ++cursor;
      auto bboAskDenominatorIndicator = ParseChar(Beam::Store(cursor));
      auto bboAskPrice = ParseMoney(PRICE_DIGITS, bboAskDenominatorIndicator,
        Beam::Store(cursor));
      auto bboAskSize = LOT_SIZE * ParseNumeric(LOT_DIGITS,
        Beam::Store(cursor));
      BboQuote bboQuote{Quote{bboBidPrice, bboBidSize, Side::BID},
        Quote{bboAskPrice, bboAskSize, Side::ASK}, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    }
    MarketQuote marketQuote{market, bid, ask, timestamp};
    m_marketDataFeedClient->PublishMarketQuote(
      SecurityMarketQuote{marketQuote, security});
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleShortFormTradeMessage(const CtaMessage& message) {}

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleLongFormTradeMessage(const CtaMessage& message) {
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Dispatch(const CtaMessage& message) {
    const auto LONG_QUOTE_LENGTH = 123;
    const auto LONG_TRADE_LENGTH = 103;
    const auto SHORT_QUOTE_LENGTH = 79;
    const auto SHORT_TRADE_LENGTH = 65;
    if(message.m_category == 'E') {
      if(message.m_type == 'B') {
        if(message.m_dataLength >= LONG_QUOTE_LENGTH) {
          HandleLongFormMarketQuoteMessage(message);
        } else if(message.m_dataLength == LONG_TRADE_LENGTH) {
          HandleLongFormTradeMessage(message);
        }
      } else if(message.m_type == 'D') {
        if(message.m_dataLength >= SHORT_QUOTE_LENGTH) {
          HandleShortFormMarketQuoteMessage(message);
        }
      } else if(message.m_type == 'P') {
        if(message.m_dataLength == SHORT_TRADE_LENGTH) {
          HandleShortFormTradeMessage(message);
        }
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
          std::cout << std::string{message.m_data, message.m_dataLength} <<
            std::endl;
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
