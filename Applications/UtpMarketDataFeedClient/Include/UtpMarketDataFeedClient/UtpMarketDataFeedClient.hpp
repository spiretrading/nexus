#ifndef NEXUS_UTP_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_UTP_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "UtpMarketDataFeedClient/UtpConfiguration.hpp"
#include "UtpMarketDataFeedClient/UtpMessage.hpp"

namespace Nexus::MarketDataService {

  /**
   * Parses packets from a UTP market data feed.
   * @param <M> The type of MarketDataFeedClient used to update the
   *        MarketDataServer.
   * @param <P> The type of client receiving messages.
   */
  template<typename M, typename P>
  class UtpMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::GetTryDereferenceType<M>;

      /** The type of client receiving messages. */
      using ProtocolClient = Beam::GetTryDereferenceType<P>;

      /**
       * Constructs a UtpMarketDataFeedClient.
       * @param config The configuration to use.
       * @param marketDataFeedClient Initializes the MarketDataFeedClient.
       * @param protocolClient The client receiving messages.
       */
      template<typename MF, typename PF>
      UtpMarketDataFeedClient(UtpConfiguration config,
        MF&& marketDataFeedClient, PF&& protocolClient);

      ~UtpMarketDataFeedClient();

      void Close();

    private:
      UtpConfiguration m_config;
      Beam::GetOptionalLocalPtr<M> m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<P> m_protocolClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      UtpMarketDataFeedClient(const UtpMarketDataFeedClient&) = delete;
      UtpMarketDataFeedClient& operator =(
        const UtpMarketDataFeedClient&) = delete;
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

  template<typename M, typename P>
  template<typename MF, typename PF>
  UtpMarketDataFeedClient<M, P>::UtpMarketDataFeedClient(
      UtpConfiguration config, MF&& marketDataFeedClient, PF&& protocolClient)
      try : m_config(std::move(config)),
            m_marketDataFeedClient(std::forward<MF>(marketDataFeedClient)),
            m_protocolClient(std::forward<PF>(protocolClient)),
            m_readLoopRoutine(Beam::Routines::Spawn(
              std::bind(&UtpMarketDataFeedClient::ReadLoop, this))) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to initialize the UTP market data feed client."));
  }

  template<typename M, typename P>
  UtpMarketDataFeedClient<M, P>::~UtpMarketDataFeedClient() {
    Close();
  }

  template<typename M, typename P>
  void UtpMarketDataFeedClient<M, P>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_protocolClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.Close();
  }

  template<typename M, typename P>
  template<typename T>
  T UtpMarketDataFeedClient<M, P>::ParseNumeric(Beam::Out<const char*> cursor) {
    auto value = Beam::FromBigEndian(*reinterpret_cast<const T*>(*cursor));
    *cursor += sizeof(T);
    return value;
  }

  template<typename M, typename P>
  Money UtpMarketDataFeedClient<M, P>::ParseMoneyShort(
      Beam::Out<const char*> cursor) {
    auto value = ParseNumeric<std::uint16_t>(Beam::Store(cursor));
    return (value * Money::ONE) / 100;
  }

  template<typename M, typename P>
  Money UtpMarketDataFeedClient<M, P>::ParseMoneyLong(
      Beam::Out<const char*> cursor) {
    auto multiplier = Quantity(1) / 1000000;
    auto value = ParseNumeric<std::uint64_t>(Beam::Store(cursor));
    return Money{multiplier * value};
  }

  template<typename M, typename P>
  std::string UtpMarketDataFeedClient<M, P>::ParseAlphanumeric(std::size_t size,
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
  MarketCode UtpMarketDataFeedClient<M, P>::ParseMarket(
      std::uint8_t identifier) {
    return m_config.m_marketCodes[identifier];
  }

  template<typename M, typename P>
  boost::posix_time::ptime UtpMarketDataFeedClient<M, P>::ParseTimestamp(
      std::uint64_t timestamp) {
    static const auto EPOCH_TIME = boost::posix_time::ptime(
      boost::gregorian::date(1970, 1, 1));
    return EPOCH_TIME + boost::posix_time::microseconds(timestamp / 1000);
  }

  template<typename M, typename P>
  void UtpMarketDataFeedClient<M, P>::HandleShortFormMarketQuoteMessage(
      const UtpMessage& message) {
    constexpr auto SYMBOL_SIZE = 5;
    constexpr auto LOT_SIZE = 100;
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
    auto security = Security(symbol, m_config.m_market, m_config.m_country);
    auto bid = Quote(bidPrice, bidSize, Side::BID);
    auto ask = Quote(askPrice, askSize, Side::ASK);
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
      auto bboBid = Quote(bboBidPrice, bboBidSize, Side::BID);
      auto bboAsk = Quote(bboAskPrice, bboAskSize, Side::ASK);
      auto bboQuote = BboQuote(bboBid, bboAsk, timestamp);
      m_marketDataFeedClient->Publish(SecurityBboQuote(bboQuote, security));
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
      auto bboBid = Quote(bboBidPrice, bboBidSize, Side::BID);
      auto bboAsk = Quote(bboAskPrice, bboAskSize, Side::ASK);
      auto bboQuote = BboQuote(bboBid, bboAsk, timestamp);
      m_marketDataFeedClient->Publish(SecurityBboQuote(bboQuote, security));
    } else if(bboIndicator == '4') {
      auto bboQuote = BboQuote(bid, ask, timestamp);
      m_marketDataFeedClient->Publish(SecurityBboQuote(bboQuote, security));
    }
    auto marketQuote = MarketQuote(market, bid, ask, timestamp);
    m_marketDataFeedClient->Publish(SecurityMarketQuote(marketQuote, security));
  }

  template<typename M, typename P>
  void UtpMarketDataFeedClient<M, P>::HandleLongFormMarketQuoteMessage(
      const UtpMessage& message) {
    constexpr auto SYMBOL_SIZE = 11;
    constexpr auto LOT_SIZE = 100;
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
    auto security = Security(symbol, m_config.m_market, m_config.m_country);
    auto bid = Quote(bidPrice, bidSize, Side::BID);
    auto ask = Quote(askPrice, askSize, Side::ASK);
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
      auto bboBid = Quote(bboBidPrice, bboBidSize, Side::BID);
      auto bboAsk = Quote(bboAskPrice, bboAskSize, Side::ASK);
      auto bboQuote = BboQuote(bboBid, bboAsk, timestamp);
      m_marketDataFeedClient->Publish(SecurityBboQuote(bboQuote, security));
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
      auto bboBid = Quote(bboBidPrice, bboBidSize, Side::BID);
      auto bboAsk = Quote(bboAskPrice, bboAskSize, Side::ASK);
      auto bboQuote = BboQuote(bboBid, bboAsk, timestamp);
      m_marketDataFeedClient->Publish(SecurityBboQuote(bboQuote, security));
    } else if(bboIndicator == '4') {
      auto bboQuote = BboQuote(bid, ask, timestamp);
      m_marketDataFeedClient->Publish(SecurityBboQuote(bboQuote, security));
    }
    auto marketQuote = MarketQuote(market, bid, ask, timestamp);
    m_marketDataFeedClient->Publish(SecurityMarketQuote(marketQuote, security));
  }

  template<typename M, typename P>
  void UtpMarketDataFeedClient<M, P>::HandleShortFormTradeReportMessage(
      const UtpMessage& message) {
    constexpr auto CONDITION_CODE_SIZE = 4;
    constexpr auto SYMBOL_SIZE = 5;
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
    auto security = Security(symbol, m_config.m_market, m_config.m_country);
    auto condition = TimeAndSale::Condition(
      TimeAndSale::Condition::Type::REGULAR, conditionCode);
    auto timeAndSale = TimeAndSale(timestamp, price, volume, condition,
      market.GetData());
    m_marketDataFeedClient->Publish(SecurityTimeAndSale(timeAndSale, security));
  }

  template<typename M, typename P>
  void UtpMarketDataFeedClient<M, P>::HandleLongFormTradeReportMessage(
      const UtpMessage& message) {
    constexpr auto SYMBOL_SIZE = 11;
    constexpr auto CONDITION_CODE_SIZE = 4;
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
    auto security = Security(symbol, m_config.m_market, m_config.m_country);
    auto condition = TimeAndSale::Condition(
      TimeAndSale::Condition::Type::REGULAR, conditionCode);
    auto timeAndSale = TimeAndSale(timestamp, price, volume, condition,
      market.GetData());
    m_marketDataFeedClient->Publish(SecurityTimeAndSale(timeAndSale, security));
  }

  template<typename M, typename P>
  void UtpMarketDataFeedClient<M, P>::Dispatch(const UtpMessage& message) {
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

  template<typename M, typename P>
  void UtpMarketDataFeedClient<M, P>::ReadLoop() {
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

#endif
