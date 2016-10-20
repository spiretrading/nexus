#ifndef NEXUS_UTPMARKETDATAFEEDCLIENT_HPP
#define NEXUS_UTPMARKETDATAFEEDCLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "UtpMarketDataFeedClient/UtpConfiguration.hpp"
#include "UtpMarketDataFeedClient/UtpMessage.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class UtpMarketDataFeedClient
      \brief Parses packets from a UTP market data feed.
      \tparam MarketDataFeedClientType The type of MarketDataFeedClient used to
              update the MarketDataServer.
      \tparam ProtocolClientType The type of client receiving messages.
   */
  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  class UtpMarketDataFeedClient : private boost::noncopyable {
    public:

      //! The type of MarketDataFeedClient used to update the MarketDataServer.
      using MarketDataFeedClient =
        Beam::GetTryDereferenceType<MarketDataFeedClientType>;

      //! The type of client receiving messages.
      using ProtocolClient = Beam::GetTryDereferenceType<ProtocolClientType>;

      //! Constructs a UtpMarketDataFeedClient.
      /*!
        \param config The configuration to use.
        \param marketDataFeedClient Initializes the MarketDataFeedClient.
        \param protocolClient The client receiving messages.
      */
      template<typename MarketDataFeedClientForward,
        typename ProtocolClientForward>
      UtpMarketDataFeedClient(const UtpConfiguration& config,
        MarketDataFeedClientForward&& marketDataFeedClient,
        ProtocolClientForward&& protocolClient);

      ~UtpMarketDataFeedClient();

      void Open();

      void Close();

    private:
      UtpConfiguration m_config;
      Beam::GetOptionalLocalPtr<MarketDataFeedClientType>
        m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<ProtocolClientType> m_protocolClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      char ParseChar(Beam::Out<const char*> cursor);
      Quantity ParseNumeric(std::size_t length, Beam::Out<const char*> cursor);
      Money ParseMoney(std::size_t length, char denominatorType,
        Beam::Out<const char*> cursor);
      std::string ParseAlphanumeric(std::size_t size,
        Beam::Out<const char*> cursor);
      MarketCode ParseMarket(std::uint8_t identifier);
      boost::posix_time::ptime ParseTimestamp(std::uint64_t timestamp);
      void HandleShortFormMarketQuoteMessage(const UtpMessage& message);
      void HandleLongFormMarketQuoteMessage(const UtpMessage& message);
      void HandleShortFormTradeReportMessage(const UtpMessage& message);
      void HandleLongFormTradeReportMessage(const UtpMessage& message);
      void Dispatch(const UtpMessage& message);
      void ReadLoop();
  };

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  template<typename MarketDataFeedClientForward, typename ProtocolClientForward>
  UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      UtpMarketDataFeedClient(const UtpConfiguration& config,
      MarketDataFeedClientForward&& marketDataFeedClient,
      ProtocolClientForward&& protocolClient)
      : m_config{config},
        m_marketDataFeedClient{std::forward<MarketDataFeedClientForward>(
          marketDataFeedClient)},
        m_protocolClient{std::forward<ProtocolClientForward>(protocolClient)} {}

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ~UtpMarketDataFeedClient() {
    Close();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataFeedClient->Open();
      m_protocolClient->Open();
      m_readLoopRoutine = Beam::Routines::Spawn(
        std::bind(&UtpMarketDataFeedClient::ReadLoop, this));
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Shutdown() {
    m_protocolClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.SetClosed();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  char UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ParseChar(Beam::Out<const char*> cursor) {
    auto value = **cursor;
    ++*cursor;
    return value;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  Quantity UtpMarketDataFeedClient<MarketDataFeedClientType,
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
  Money UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ParseMoney(std::size_t length, char denominatorType,
      Beam::Out<const char*> cursor) {
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
  std::string UtpMarketDataFeedClient<MarketDataFeedClientType,
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
  MarketCode UtpMarketDataFeedClient<
      MarketDataFeedClientType, ProtocolClientType>::ParseMarket(
      std::uint8_t identifier) {
    return m_config.m_marketCodes[identifier];
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  boost::posix_time::ptime UtpMarketDataFeedClient<
      MarketDataFeedClientType, ProtocolClientType>::ParseTimestamp(
      std::uint64_t timestamp) {
/*
    const auto BASE = 95;
    std::uint64_t microseconds = 0;
    for(auto digit : code) {
      microseconds = BASE * microseconds + (digit - ' ');
    }
    return m_config.m_timeOrigin +
      boost::posix_time::microseconds(microseconds);
*/
    return {};
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleShortFormMarketQuoteMessage(const UtpMessage& message) {
/*
    const auto SYMBOL_SIZE = 5;
    const auto LOT_DIGITS = 2;
    const auto LOT_SIZE = 100;
    const auto PRICE_DIGITS = 6;
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(message.m_sipTimestamp);
    auto symbol = ParseAlphanumeric(SYMBOL_SIZE, Beam::Store(cursor));
    ++cursor;
    ++cursor;
    ++cursor;
    ++cursor;
    auto bidDenominatorType = ParseChar(Beam::Store(cursor));
    auto bidPrice = ParseMoney(PRICE_DIGITS, bidDenominatorType,
      Beam::Store(cursor));
    auto bidSize = LOT_SIZE * ParseNumeric(LOT_DIGITS, Beam::Store(cursor));
    auto askDenominatorType = ParseChar(Beam::Store(cursor));
    auto askPrice = ParseMoney(PRICE_DIGITS, askDenominatorType,
      Beam::Store(cursor));
    auto askSize = LOT_SIZE * ParseNumeric(LOT_DIGITS, Beam::Store(cursor));
    auto bboIndicator = ParseChar(Beam::Store(cursor));
    auto market = ParseMarket(message.m_marketCenterOriginatorId);
    Security security{symbol, m_config.m_market, m_config.m_country};
    Quote bid{bidPrice, bidSize, Side::BID};
    Quote ask{askPrice, askSize, Side::ASK};
    if(bboIndicator == '2') {
      ++cursor;
      ++cursor;
      ++cursor;
      ++cursor;
      auto bboBidDenominatorType = ParseChar(Beam::Store(cursor));
      auto bboBidPrice = ParseMoney(PRICE_DIGITS, bboBidDenominatorType,
        Beam::Store(cursor));
      auto bboBidSize = LOT_SIZE * ParseNumeric(LOT_DIGITS,
        Beam::Store(cursor));
      ++cursor;
      ++cursor;
      auto bboAskDenominatorType = ParseChar(Beam::Store(cursor));
      auto bboAskPrice = ParseMoney(PRICE_DIGITS, bboAskDenominatorType,
        Beam::Store(cursor));
      auto bboAskSize = LOT_SIZE * ParseNumeric(LOT_DIGITS,
        Beam::Store(cursor));
      Quote bboBid{bboBidPrice, bboBidSize, Side::BID};
      Quote bboAsk{bboAskPrice, bboAskSize, Side::ASK};
      BboQuote bboQuote{bboBid, bboAsk, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(bboIndicator == '3') {
      const auto LONG_FORM_PRICE_DIGITS = 10;
      const auto LONG_FORM_LOT_DIGITS = 7;
      ++cursor;
      ++cursor;
      ++cursor;
      ++cursor;
      auto bboBidDenominatorType = ParseChar(Beam::Store(cursor));
      auto bboBidPrice = ParseMoney(LONG_FORM_PRICE_DIGITS,
        bboBidDenominatorType, Beam::Store(cursor));
      auto bboBidSize = LOT_SIZE * ParseNumeric(LONG_FORM_LOT_DIGITS,
        Beam::Store(cursor));
      ++cursor;
      ++cursor;
      auto bboAskDenominatorType = ParseChar(Beam::Store(cursor));
      auto bboAskPrice = ParseMoney(LONG_FORM_PRICE_DIGITS,
        bboAskDenominatorType, Beam::Store(cursor));
      auto bboAskSize = LOT_SIZE * ParseNumeric(LONG_FORM_LOT_DIGITS,
        Beam::Store(cursor));
      Quote bboBid{bboBidPrice, bboBidSize, Side::BID};
      Quote bboAsk{bboAskPrice, bboAskSize, Side::ASK};
      BboQuote bboQuote{bboBid, bboAsk, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(bboIndicator == '4') {
      BboQuote bboQuote{bid, ask, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    }
    MarketQuote marketQuote{market, bid, ask, timestamp};
    m_marketDataFeedClient->PublishMarketQuote(
      SecurityMarketQuote{marketQuote, security});
*/
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleLongFormMarketQuoteMessage(const UtpMessage& message) {
/*
    const auto SYMBOL_SIZE = 11;
    const auto LOT_DIGITS = 7;
    const auto LOT_SIZE = 100;
    const auto PRICE_DIGITS = 10;
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(message.m_sipTimestamp);
    auto symbol = ParseAlphanumeric(SYMBOL_SIZE, Beam::Store(cursor));
    ++cursor;
    ++cursor;
    ++cursor;
    ++cursor;
    ++cursor;
    auto bidDenominatorType = ParseChar(Beam::Store(cursor));
    auto bidPrice = ParseMoney(PRICE_DIGITS, bidDenominatorType,
      Beam::Store(cursor));
    auto bidSize = LOT_SIZE * ParseNumeric(LOT_DIGITS, Beam::Store(cursor));
    auto askDenominatorType = ParseChar(Beam::Store(cursor));
    auto askPrice = ParseMoney(PRICE_DIGITS, askDenominatorType,
      Beam::Store(cursor));
    auto askSize = LOT_SIZE * ParseNumeric(LOT_DIGITS, Beam::Store(cursor));
    cursor += 3;
    auto bboIndicator = ParseChar(Beam::Store(cursor));
    auto market = ParseMarket(message.m_marketCenterOriginatorId);
    Security security{symbol, m_config.m_market, m_config.m_country};
    Quote bid{bidPrice, bidSize, Side::BID};
    Quote ask{askPrice, askSize, Side::ASK};
    if(bboIndicator == '2') {
      ++cursor;
      ++cursor;
      ++cursor;
      ++cursor;
      auto bboBidDenominatorType = ParseChar(Beam::Store(cursor));
      auto bboBidPrice = ParseMoney(PRICE_DIGITS, bboBidDenominatorType,
        Beam::Store(cursor));
      auto bboBidSize = LOT_SIZE * ParseNumeric(LOT_DIGITS,
        Beam::Store(cursor));
      ++cursor;
      ++cursor;
      auto bboAskDenominatorType = ParseChar(Beam::Store(cursor));
      auto bboAskPrice = ParseMoney(PRICE_DIGITS, bboAskDenominatorType,
        Beam::Store(cursor));
      auto bboAskSize = LOT_SIZE * ParseNumeric(LOT_DIGITS,
        Beam::Store(cursor));
      Quote bboBid{bboBidPrice, bboBidSize, Side::BID};
      Quote bboAsk{bboAskPrice, bboAskSize, Side::ASK};
      BboQuote bboQuote{bboBid, bboAsk, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(bboIndicator == '3') {
      const auto LONG_FORM_PRICE_DIGITS = 10;
      const auto LONG_FORM_LOT_DIGITS = 7;
      ++cursor;
      ++cursor;
      ++cursor;
      ++cursor;
      auto bboBidDenominatorType = ParseChar(Beam::Store(cursor));
      auto bboBidPrice = ParseMoney(LONG_FORM_PRICE_DIGITS,
        bboBidDenominatorType, Beam::Store(cursor));
      auto bboBidSize = LOT_SIZE * ParseNumeric(LONG_FORM_LOT_DIGITS,
        Beam::Store(cursor));
      ++cursor;
      ++cursor;
      auto bboAskDenominatorType = ParseChar(Beam::Store(cursor));
      auto bboAskPrice = ParseMoney(LONG_FORM_PRICE_DIGITS,
        bboAskDenominatorType, Beam::Store(cursor));
      auto bboAskSize = LOT_SIZE * ParseNumeric(LONG_FORM_LOT_DIGITS,
        Beam::Store(cursor));
      Quote bboBid{bboBidPrice, bboBidSize, Side::BID};
      Quote bboAsk{bboAskPrice, bboAskSize, Side::ASK};
      BboQuote bboQuote{bboBid, bboAsk, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(bboIndicator == '4') {
      BboQuote bboQuote{bid, ask, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    }
    MarketQuote marketQuote{market, bid, ask, timestamp};
    m_marketDataFeedClient->PublishMarketQuote(
      SecurityMarketQuote{marketQuote, security});
*/
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleShortFormTradeReportMessage(const UtpMessage& message) {
/*
    const auto SYMBOL_SIZE = 5;
    const auto PRICE_DIGITS = 6;
    const auto VOLUME_DIGITS = 6;
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(message.m_sipTimestamp);
    auto symbol = ParseAlphanumeric(SYMBOL_SIZE, Beam::Store(cursor));
    auto conditionCode = ParseAlphanumeric(1, Beam::Store(cursor));
    auto priceDenominatorType = ParseChar(Beam::Store(cursor));
    auto price = ParseMoney(PRICE_DIGITS, priceDenominatorType,
      Beam::Store(cursor));
    auto volume = ParseNumeric(VOLUME_DIGITS, Beam::Store(cursor));
    auto market = ParseMarket(message.m_marketCenterOriginatorId);
    Security security{symbol, m_config.m_market, m_config.m_country};
    TimeAndSale::Condition condition{
      TimeAndSale::Condition::Type::REGULAR, conditionCode};
    TimeAndSale timeAndSale{timestamp, price, volume, condition,
      market.GetData()};
    m_marketDataFeedClient->PublishTimeAndSale(
      SecurityTimeAndSale{timeAndSale, security});
*/
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleLongFormTradeReportMessage(const UtpMessage& message) {
/*
    const auto SYMBOL_SIZE = 11;
    const auto CONDITION_SIZE = 4;
    const auto PRICE_DIGITS = 10;
    const auto VOLUME_DIGITS = 9;
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(message.m_sipTimestamp);
    auto symbol = ParseAlphanumeric(SYMBOL_SIZE, Beam::Store(cursor));
    ++cursor;
    auto conditionCode = ParseAlphanumeric(CONDITION_SIZE, Beam::Store(cursor));
    cursor += 2;
    auto priceDenominatorType = ParseChar(Beam::Store(cursor));
    auto price = ParseMoney(PRICE_DIGITS, priceDenominatorType,
      Beam::Store(cursor));
    cursor += 3;
    auto volume = ParseNumeric(VOLUME_DIGITS, Beam::Store(cursor));
    auto market = ParseMarket(message.m_marketCenterOriginatorId);
    Security security{symbol, m_config.m_market, m_config.m_country};
    TimeAndSale::Condition condition{
      TimeAndSale::Condition::Type::REGULAR, conditionCode};
    TimeAndSale timeAndSale{timestamp, price, volume, condition,
      market.GetData()};
    m_marketDataFeedClient->PublishTimeAndSale(
      SecurityTimeAndSale{timeAndSale, security});
*/
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Dispatch(const UtpMessage& message) {
    if(message.m_category == 'Q') {
      if(message.m_type == 'E') {
        HandleShortFormMarketQuoteMessage(message);
      } else if(message.m_type == 'F') {
        HandleLongFormMarketQuoteMessage(message);
      }
    } else if(message.m_category == 'T') {
      if(message.m_type == 'A') {
        HandleShortFormTradeReportMessage(message);
      } else if(message.m_type == 'W') {
        HandleLongFormTradeReportMessage(message);
      }
    }
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ReadLoop() {
    while(true) {
      try {
        auto message = m_protocolClient->Read();
        Dispatch(message);
      } catch(const Beam::IO::EndOfFileException&) {
        break;
      }
    }
  }
}
}

#endif
