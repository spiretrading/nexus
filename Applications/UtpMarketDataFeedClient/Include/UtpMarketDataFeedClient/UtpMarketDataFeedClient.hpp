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
      template<typename T>
      T ParseNumeric(Beam::Out<const char*> cursor);
      Money ParseMoneyShort(Beam::Out<const char*> cursor);
      Money ParseMoneyLong(Beam::Out<const char*> cursor);
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
  template<typename T>
  T UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ParseNumeric(Beam::Out<const char*> cursor) {
    auto value = Beam::FromBigEndian<T>(*reinterpret_cast<const T*>(*cursor));
    *cursor += sizeof(T);
    return value;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  Money UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ParseMoneyShort(Beam::Out<const char*> cursor) {
    auto value = ParseNumeric<std::uint16_t>(Beam::Store(cursor));
    return (value * Money::ONE) / 100;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  Money UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ParseMoneyLong(Beam::Out<const char*> cursor) {
    auto value = ParseNumeric<std::uint64_t>(Beam::Store(cursor));
    return Money::FromRepresentation(value);
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
    static const boost::posix_time::ptime EPOCH_TIME{
      boost::gregorian::date{1970, 1, 1}};
    return EPOCH_TIME + boost::posix_time::microseconds(timestamp / 1000);
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleShortFormMarketQuoteMessage(const UtpMessage& message) {
    const auto SYMBOL_SIZE = 5;
    const auto LOT_SIZE = 100;
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(message.m_sipTimestamp);
    auto symbol = ParseAlphanumeric(SYMBOL_SIZE, Beam::Store(cursor));
    auto bidPrice = ParseMoneyShort(Beam::Store(cursor));
    auto bidSize = LOT_SIZE * ParseNumeric<std::uint16_t>(Beam::Store(cursor));
    auto askPrice = ParseMoneyShort(Beam::Store(cursor));
    auto askSize = LOT_SIZE * ParseNumeric<std::uint16_t>(Beam::Store(cursor));
    ++cursor;
    ++cursor;
    ++cursor;
    ++cursor;
    auto bboIndicator = ParseNumeric<std::uint8_t>(Beam::Store(cursor));
    ++cursor;
    auto market = ParseMarket(message.m_marketCenterOriginatorId);
    Security security{symbol, m_config.m_market, m_config.m_country};
    Quote bid{bidPrice, bidSize, Side::BID};
    Quote ask{askPrice, askSize, Side::ASK};
    if(bboIndicator == '2') {
      ++cursor;
      ++cursor;
      auto bboBidPrice = ParseMoneyShort(Beam::Store(cursor));
      auto bboBidSize = LOT_SIZE * ParseNumeric<std::uint16_t>(
        Beam::Store(cursor));
      ++cursor;
      auto bboAskPrice = ParseMoneyShort(Beam::Store(cursor));
      auto bboAskSize = LOT_SIZE * ParseNumeric<std::uint16_t>(
        Beam::Store(cursor));
      Quote bboBid{bboBidPrice, bboBidSize, Side::BID};
      Quote bboAsk{bboAskPrice, bboAskSize, Side::ASK};
      BboQuote bboQuote{bboBid, bboAsk, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(bboIndicator == '3') {
      ++cursor;
      ++cursor;
      auto bboBidPrice = ParseMoneyLong(Beam::Store(cursor));
      auto bboBidSize = LOT_SIZE * ParseNumeric<std::uint32_t>(
        Beam::Store(cursor));
      ++cursor;
      auto bboAskPrice = ParseMoneyLong(Beam::Store(cursor));
      auto bboAskSize = LOT_SIZE * ParseNumeric<std::uint32_t>(
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
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleLongFormMarketQuoteMessage(const UtpMessage& message) {
    const auto SYMBOL_SIZE = 11;
    const auto LOT_SIZE = 100;
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(message.m_sipTimestamp);
    cursor += 8;
    auto symbol = ParseAlphanumeric(SYMBOL_SIZE, Beam::Store(cursor));
    auto bidPrice = ParseMoneyLong(Beam::Store(cursor));
    auto bidSize = LOT_SIZE * ParseNumeric<std::uint32_t>(Beam::Store(cursor));
    auto askPrice = ParseMoneyLong(Beam::Store(cursor));
    auto askSize = LOT_SIZE * ParseNumeric<std::uint32_t>(Beam::Store(cursor));
    ++cursor;
    ++cursor;
    ++cursor;
    ++cursor;
    auto bboIndicator = ParseNumeric<std::uint8_t>(Beam::Store(cursor));
    ++cursor;
    ++cursor;
    auto market = ParseMarket(message.m_marketCenterOriginatorId);
    Security security{symbol, m_config.m_market, m_config.m_country};
    Quote bid{bidPrice, bidSize, Side::BID};
    Quote ask{askPrice, askSize, Side::ASK};
    if(bboIndicator == '2') {
      ++cursor;
      ++cursor;
      auto bboBidPrice = ParseMoneyShort(Beam::Store(cursor));
      auto bboBidSize = LOT_SIZE * ParseNumeric<std::uint16_t>(
        Beam::Store(cursor));
      ++cursor;
      auto bboAskPrice = ParseMoneyShort(Beam::Store(cursor));
      auto bboAskSize = LOT_SIZE * ParseNumeric<std::uint16_t>(
        Beam::Store(cursor));
      Quote bboBid{bboBidPrice, bboBidSize, Side::BID};
      Quote bboAsk{bboAskPrice, bboAskSize, Side::ASK};
      BboQuote bboQuote{bboBid, bboAsk, timestamp};
      m_marketDataFeedClient->PublishBboQuote(
        SecurityBboQuote{bboQuote, security});
    } else if(bboIndicator == '3') {
      ++cursor;
      ++cursor;
      auto bboBidPrice = ParseMoneyLong(Beam::Store(cursor));
      auto bboBidSize = LOT_SIZE * ParseNumeric<std::uint32_t>(
        Beam::Store(cursor));
      ++cursor;
      auto bboAskPrice = ParseMoneyLong(Beam::Store(cursor));
      auto bboAskSize = LOT_SIZE * ParseNumeric<std::uint32_t>(
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
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleShortFormTradeReportMessage(const UtpMessage& message) {
    const auto CONDITION_CODE_SIZE = 4;
    const auto SYMBOL_SIZE = 5;
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(message.m_sipTimestamp);
    cursor += 8;
    auto symbol = ParseAlphanumeric(SYMBOL_SIZE, Beam::Store(cursor));
    cursor += 8;
    auto price = ParseMoneyShort(Beam::Store(cursor));
    auto volume = ParseNumeric<std::uint16_t>(Beam::Store(cursor));
    auto market = ParseMarket(message.m_marketCenterOriginatorId);
    auto conditionCode = ParseAlphanumeric(CONDITION_CODE_SIZE,
      Beam::Store(cursor));
    Security security{symbol, m_config.m_market, m_config.m_country};
    TimeAndSale::Condition condition{
      TimeAndSale::Condition::Type::REGULAR, conditionCode};
    TimeAndSale timeAndSale{timestamp, price, volume, condition,
      market.GetData()};
    m_marketDataFeedClient->PublishTimeAndSale(
      SecurityTimeAndSale{timeAndSale, security});
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void UtpMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleLongFormTradeReportMessage(const UtpMessage& message) {
    const auto SYMBOL_SIZE = 11;
    const auto CONDITION_CODE_SIZE = 4;
    auto cursor = message.m_data;
    cursor += 8;
    auto timestamp = ParseTimestamp(message.m_sipTimestamp);
    auto symbol = ParseAlphanumeric(SYMBOL_SIZE, Beam::Store(cursor));
    cursor += 8;
    auto price = ParseMoneyLong(Beam::Store(cursor));
    auto volume = ParseNumeric<std::uint32_t>(Beam::Store(cursor));
    auto conditionCode = ParseAlphanumeric(
      CONDITION_CODE_SIZE, Beam::Store(cursor));
    auto market = ParseMarket(message.m_marketCenterOriginatorId);
    Security security{symbol, m_config.m_market, m_config.m_country};
    TimeAndSale::Condition condition{
      TimeAndSale::Condition::Type::REGULAR, conditionCode};
    TimeAndSale timeAndSale{timestamp, price, volume, condition,
      market.GetData()};
    m_marketDataFeedClient->PublishTimeAndSale(
      SecurityTimeAndSale{timeAndSale, security});
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
