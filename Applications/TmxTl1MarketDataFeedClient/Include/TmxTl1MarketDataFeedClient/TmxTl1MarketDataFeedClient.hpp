#ifndef NEXUS_TMXTL1MARKETDATAFEEDCLIENT_HPP
#define NEXUS_TMXTL1MARKETDATAFEEDCLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1Configuration.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1ServiceAccessClient.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class TmxTl1MarketDataFeedClient
      \brief Parses packets from the TMX TL1 feed.
      \tparam MarketDataFeedClientType The type of MarketDataFeedClient used to
              update the MarketDataServer.
      \tparam ServiceAccessClientType The type of service access client
              receiving messages.
   */
  template<typename MarketDataFeedClientType, typename ServiceAccessClientType>
  class TmxTl1MarketDataFeedClient : private boost::noncopyable {
    public:

      //! The type of MarketDataFeedClient used to update the MarketDataServer.
      using MarketDataFeedClient =
        Beam::GetTryDereferenceType<MarketDataFeedClientType>;

      //! The type of channel receiving the market data feed.
      using ServiceAccessClient =
        Beam::GetTryDereferenceType<ServiceAccessClientType>;

      //! Constructs a TmxTl1MarketDataFeedClient.
      /*!
        \param config The configuration to use.
        \param marketDataFeedClient Initializes the MarketDataFeedClient.
        \param serviceAccessClient The service access client receiving messages.
      */
      template<typename MarketDataFeedClientForward,
        typename ServiceAccessClientForward>
      TmxTl1MarketDataFeedClient(const TmxTl1Configuration& config,
        MarketDataFeedClientForward&& marketDataFeedClient,
        ServiceAccessClientForward&& serviceAccessClient);

      ~TmxTl1MarketDataFeedClient();

      void Open();

      void Close();

    private:
      TmxTl1Configuration m_config;
      Beam::GetOptionalLocalPtr<MarketDataFeedClientType>
        m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<ServiceAccessClientType> m_serviceAccessClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      boost::optional<Money> ParseMoney(const char* token, int integralSize,
        int fractionalSize);
      boost::optional<int> ParseQuantity(const char* token, int size);
      boost::optional<boost::posix_time::ptime> ParseTimestamp(
        const char* token);
      void HandleEquityQuoteMessage(const StampProtocol::StampPacket& message);
      void ReadLoop();
  };

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType>
  template<typename MarketDataFeedClientForward,
    typename ServiceAccessClientForward>
  TmxTl1MarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType>::TmxTl1MarketDataFeedClient(
      const TmxTl1Configuration& config,
      MarketDataFeedClientForward&& marketDataFeedClient,
      ServiceAccessClientForward&& serviceAccessClient)
      : m_config(config),
        m_marketDataFeedClient(std::forward<MarketDataFeedClientForward>(
          marketDataFeedClient)),
        m_serviceAccessClient(std::forward<ServiceAccessClientForward>(
          serviceAccessClient)) {}

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType>
  TmxTl1MarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType>::~TmxTl1MarketDataFeedClient() {
    Close();
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType>
  void TmxTl1MarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataFeedClient->Open();
      m_serviceAccessClient->Open();
      m_readLoopRoutine = Beam::Routines::Spawn(
        std::bind(&TmxTl1MarketDataFeedClient::ReadLoop, this));
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType>
  void TmxTl1MarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType>
  void TmxTl1MarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType>::Shutdown() {
    m_serviceAccessClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.SetClosed();
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType>
  boost::optional<boost::posix_time::ptime> TmxTl1MarketDataFeedClient<
      MarketDataFeedClientType, ServiceAccessClientType>::ParseTimestamp(
      const char* token) {
    std::string value(token, 20);
    auto y = boost::lexical_cast<int>(value.substr(0, 4));
    auto m = boost::lexical_cast<int>(value.substr(4, 2));
    auto d = boost::lexical_cast<int>(value.substr(6, 2));
    auto hr = boost::lexical_cast<int>(value.substr(8, 2));
    auto mn = boost::lexical_cast<int>(value.substr(10, 2));
    auto sec = boost::lexical_cast<int>(value.substr(12, 2));
    auto mill = boost::lexical_cast<int>(value.substr(14, 2));
    boost::posix_time::ptime timestamp(
      boost::gregorian::date(static_cast<unsigned short>(y),
      static_cast<unsigned short>(m), static_cast<unsigned short>(d)),
      boost::posix_time::hours(hr) + boost::posix_time::minutes(mn) +
      boost::posix_time::seconds(sec) +
      boost::posix_time::milliseconds(10 * mill));
    return timestamp + m_config.m_timeOffset;
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType>
  boost::optional<Money> TmxTl1MarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType>::ParseMoney(const char* token, int integralSize,
      int fractionalSize) {
    auto quantity = ParseQuantity(token, integralSize + fractionalSize);
    if(!quantity.is_initialized()) {
      return boost::none;
    }
    auto value = *quantity * Money::ONE;
    for(auto i = 0; i < fractionalSize; ++i) {
      value /= 10;
    }
    return value;
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType>
  boost::optional<int> TmxTl1MarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType>::ParseQuantity(const char* token, int size) {
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

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType>
  void TmxTl1MarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType>::HandleEquityQuoteMessage(
      const StampProtocol::StampPacket& message) {
    const auto SYMBOL_SIZE = 8;
    const auto PRICE_SIZE = 9;
    const auto PRICE_INTEGRAL_SIZE = 6;
    const auto PRICE_FRACTIONAL_SIZE = 3;
    const auto VOLUME_SIZE = 9;
    const auto TIMESTAMP_SIZE = 20;
    auto remainingSize = message.m_messageSize;
    auto token = message.m_message;
    if(remainingSize < SYMBOL_SIZE) {
      return;
    }
    std::string symbol;
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
    if(!bidPrice.is_initialized()) {
      return;
    }
    token += PRICE_SIZE;
    remainingSize -= PRICE_SIZE;
    if(remainingSize < VOLUME_SIZE) {
      return;
    }
    auto bidVolume = ParseQuantity(token, VOLUME_SIZE);
    if(!bidVolume.is_initialized()) {
      return;
    }
    token += VOLUME_SIZE;
    remainingSize -= VOLUME_SIZE;
    if(remainingSize < PRICE_SIZE) {
      return;
    }
    auto askPrice = ParseMoney(token, PRICE_INTEGRAL_SIZE,
      PRICE_FRACTIONAL_SIZE);
    if(!askPrice.is_initialized()) {
      return;
    }
    token += PRICE_SIZE;
    remainingSize -= PRICE_SIZE;
    auto askVolume = ParseQuantity(token, VOLUME_SIZE);
    if(!askVolume.is_initialized()) {
      return;
    }
    token += VOLUME_SIZE;
    remainingSize -= VOLUME_SIZE;
    if(remainingSize < TIMESTAMP_SIZE) {
      return;
    }
    auto timestamp = ParseTimestamp(token);
    if(!timestamp.is_initialized()) {
      return;
    }
    Security security(std::move(symbol), m_config.m_market, m_config.m_country);
    Quote bid(*bidPrice, *bidVolume, Side::BID);
    Quote ask(*askPrice, *askVolume, Side::ASK);
    BboQuote bbo(bid, ask, *timestamp);
    m_marketDataFeedClient->PublishBboQuote(SecurityBboQuote(bbo, security));
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType>
  void TmxTl1MarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType>::ReadLoop() {
    while(true) {
      Beam::DelayPtr<StampProtocol::StampPacket> message;
      try {
        message.Initialize(m_serviceAccessClient->Read());
      } catch(Beam::IO::NotConnectedException&) {
        break;
      } catch(Beam::IO::EndOfFileException&) {
        break;
      }
      if(m_config.m_isLoggingMessages) {
        std::cout << message->m_header.m_sequenceNumber << ": " <<
          std::string(message->m_message, message->m_messageSize) << "\n";
      }
      if(message->m_header.m_messageType == "E ") {
        HandleEquityQuoteMessage(*message);
      }
    }
  }
}
}

#endif
