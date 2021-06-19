#ifndef NEXUS_TMX_TL1_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_TMX_TL1_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1Configuration.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1ServiceAccessClient.hpp"

namespace Nexus::MarketDataService {

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
      using MarketDataFeedClient = Beam::GetTryDereferenceType<M>;

      /** The type of channel receiving the market data feed. */
      using ServiceAccessClient = Beam::GetTryDereferenceType<S>;

      /**
       * Constructs a TmxTl1MarketDataFeedClient.
       * @param config The configuration to use.
       * @param marketDataFeedClient Initializes the MarketDataFeedClient.
       * @param serviceAccessClient The service access client receiving
       *        messages.
       */
      template<typename MF, typename SF>
      TmxTl1MarketDataFeedClient(TmxTl1Configuration config,
        MF&& marketDataFeedClient, SF&& serviceAccessClient);

      ~TmxTl1MarketDataFeedClient();

      void Close();

    private:
      TmxTl1Configuration m_config;
      Beam::GetOptionalLocalPtr<M> m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<S> m_serviceAccessClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      TmxTl1MarketDataFeedClient(const TmxTl1MarketDataFeedClient&) = delete;
      TmxTl1MarketDataFeedClient& operator =(
        const TmxTl1MarketDataFeedClient&) = delete;
      boost::optional<Money> ParseMoney(const char* token, int integralSize,
        int fractionalSize);
      boost::optional<int> ParseQuantity(const char* token, int size);
      boost::optional<boost::posix_time::ptime> ParseTimestamp(
        const char* token);
      void HandleEquityQuoteMessage(const StampProtocol::StampPacket& message);
      void ReadLoop();
  };

  template<typename M, typename S>
  template<typename MF, typename SF>
  TmxTl1MarketDataFeedClient<M, S>::TmxTl1MarketDataFeedClient(
      TmxTl1Configuration config, MF&& marketDataFeedClient,
      SF&& serviceAccessClient)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_config(std::move(config)),
            m_marketDataFeedClient(std::forward<MF>(marketDataFeedClient)),
            m_serviceAccessClient(std::forward<SF>(serviceAccessClient)),
            m_readLoopRoutine(Beam::Routines::Spawn(
              std::bind(&TmxTl1MarketDataFeedClient::ReadLoop, this))) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to initialize the TMX TL1 market data feed client."));
  }

  template<typename M, typename S>
  TmxTl1MarketDataFeedClient<M, S>::~TmxTl1MarketDataFeedClient() {
    Close();
  }

  template<typename M, typename S>
  void TmxTl1MarketDataFeedClient<M, S>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_serviceAccessClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.Close();
  }

  template<typename M, typename S>
  boost::optional<boost::posix_time::ptime>
      TmxTl1MarketDataFeedClient<M, S>::ParseTimestamp(const char* token) {
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
    return timestamp + m_config.m_timeOffset;
  }

  template<typename M, typename S>
  boost::optional<Money> TmxTl1MarketDataFeedClient<M, S>::ParseMoney(
      const char* token, int integralSize, int fractionalSize) {
    auto quantity = ParseQuantity(token, integralSize + fractionalSize);
    if(!quantity) {
      return boost::none;
    }
    auto value = *quantity * Money::ONE;
    for(auto i = 0; i < fractionalSize; ++i) {
      value /= 10;
    }
    return value;
  }

  template<typename M, typename S>
  boost::optional<int> TmxTl1MarketDataFeedClient<M, S>::ParseQuantity(
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
  void TmxTl1MarketDataFeedClient<M, S>::HandleEquityQuoteMessage(
      const StampProtocol::StampPacket& message) {
    constexpr auto SYMBOL_SIZE = 12;
    constexpr auto PRICE_SIZE = 9;
    constexpr auto PRICE_INTEGRAL_SIZE = 6;
    constexpr auto PRICE_FRACTIONAL_SIZE = 3;
    constexpr auto VOLUME_SIZE = 9;
    constexpr auto TIMESTAMP_SIZE = 23;
    auto remainingSize = message.m_messageSize;
    auto token = message.m_message;
    if(remainingSize < SYMBOL_SIZE) {
      return;
    }
    auto symbol = std::string();
    auto symbolToken = token;
    while(symbol.size() < SYMBOL_SIZE && !std::isspace(*symbolToken)) {
      symbol += *symbolToken;
      ++symbolToken;
    }
    token += SYMBOL_SIZE;
    remainingSize -= SYMBOL_SIZE;
    if(remainingSize < PRICE_SIZE) {
      return;
    }
    auto bidPrice = ParseMoney(token, PRICE_INTEGRAL_SIZE,
      PRICE_FRACTIONAL_SIZE);
    if(!bidPrice) {
      return;
    }
    token += PRICE_SIZE;
    remainingSize -= PRICE_SIZE;
    if(remainingSize < VOLUME_SIZE) {
      return;
    }
    auto bidVolume = ParseQuantity(token, VOLUME_SIZE);
    if(!bidVolume) {
      return;
    }
    token += VOLUME_SIZE;
    remainingSize -= VOLUME_SIZE;
    if(remainingSize < PRICE_SIZE) {
      return;
    }
    auto askPrice = ParseMoney(token, PRICE_INTEGRAL_SIZE,
      PRICE_FRACTIONAL_SIZE);
    if(!askPrice) {
      return;
    }
    token += PRICE_SIZE;
    remainingSize -= PRICE_SIZE;
    auto askVolume = ParseQuantity(token, VOLUME_SIZE);
    if(!askVolume) {
      return;
    }
    token += VOLUME_SIZE;
    remainingSize -= VOLUME_SIZE;
    if(remainingSize < TIMESTAMP_SIZE) {
      return;
    }
    auto timestamp = ParseTimestamp(token);
    if(!timestamp) {
      return;
    }
    auto security = Security(std::move(symbol), m_config.m_market,
      m_config.m_country);
    auto bid = Quote(*bidPrice, *bidVolume, Side::BID);
    auto ask = Quote(*askPrice, *askVolume, Side::ASK);
    auto bbo = BboQuote(bid, ask, *timestamp);
    m_marketDataFeedClient->Publish(SecurityBboQuote(bbo, security));
  }

  template<typename M, typename S>
  void TmxTl1MarketDataFeedClient<M, S>::ReadLoop() {
    while(true) {
      auto message = std::optional<StampProtocol::StampPacket>();
      try {
        message.emplace(m_serviceAccessClient->Read());
      } catch(const Beam::IO::EndOfFileException&) {
        break;
      }
      if(m_config.m_isLoggingMessages) {
        std::cout << message->m_header.m_sequenceNumber << ": " <<
          message->m_header.m_messageType << " " <<
          std::string(message->m_message, message->m_messageSize) << "\n";
      }
      if(message->m_header.m_messageType == "E ") {
        HandleEquityQuoteMessage(*message);
      }
    }
  }
}

#endif
