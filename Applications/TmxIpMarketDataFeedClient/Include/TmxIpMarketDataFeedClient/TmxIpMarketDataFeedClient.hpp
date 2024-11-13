#ifndef NEXUS_TMX_IP_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_TMX_IP_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpConfiguration.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpServiceAccessClient.hpp"

namespace Nexus::MarketDataService {

  /**
   * Parses packets from the TMX Information Processor feed.
   * @param <M> The type of MarketDataFeedClient used to update the
   *            MarketDataServer.
   * @param <S> The type of service access client receiving messages.
   * @param <T> The type of TimeClient used for timestamps.
   */
  template<typename M, typename S, typename T>
  class TmxIpMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::GetTryDereferenceType<M>;

      /** The type of channel receiving the market data feed. */
      using ServiceAccessClient = Beam::GetTryDereferenceType<S>;

      /** The type of TimeClient used for timestamps. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /**
       * Constructs a TmxIpMarketDataFeedClient.
       * @param config The configuration to use.
       * @param marketDataFeedClient Initializes the MarketDataFeedClient.
       * @param serviceAccessClient The service access client receiving
       *        messages.
       * @param timeClient The TimeClient used for timestamps.
       */
      template<typename MF, typename SF, typename TF>
      TmxIpMarketDataFeedClient(TmxIpConfiguration config,
        MF&& marketDataFeedClient, SF&& serviceAccessClient, TF&& timeClient);

      ~TmxIpMarketDataFeedClient();

      void Close();

    private:
      TmxIpConfiguration m_config;
      Beam::GetOptionalLocalPtr<M> m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<S> m_serviceAccessClient;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      static std::int64_t GetBoardLotPortion(std::int64_t quantity,
        Money price);
      static std::int64_t RoundToBoardLotPortion(std::int64_t quantity,
        Money price);
      TmxIpMarketDataFeedClient(const TmxIpMarketDataFeedClient&) = delete;
      TmxIpMarketDataFeedClient& operator =(
        const TmxIpMarketDataFeedClient&) = delete;
      boost::optional<boost::posix_time::ptime> GetTimestamp(
        const StampProtocol::StampMessage& message, int index);
      const std::string& GetMpid(const std::string& brokerNumber) const;
      std::string GetOrderId(const boost::optional<std::string>& symbol,
        const boost::optional<std::string>& brokerNumber,
        const std::string& orderNumber);
      void HandleQuote(const StampProtocol::StampMessage& message);
      void HandleLastSaleTradeReport(
        const StampProtocol::StampMessage& message);
      void HandleOrderInfo(const StampProtocol::StampMessage& message);
      void HandleBookedOrder(const StampProtocol::StampMessage& message);
      void HandleCancelledOrder(const StampProtocol::StampMessage& message);
      void HandlePriceAssignedOrder(const StampProtocol::StampMessage& message);
      void HandleOrderOrCancelConfirmationReport(
        const StampProtocol::StampMessage& message);
      void HandleOrderTradeReport(const StampProtocol::StampMessage& message);
      void HandleImbalanceStatus(const StampProtocol::StampMessage& message);
      void HandleMbxAssignCalculatedOpeningPrice(
        const StampProtocol::StampMessage& message);
      void HandleMbxAssignLimit(const StampProtocol::StampMessage& message);
      void HandleMbxMessage(const StampProtocol::StampMessage& message);
      void HandleSymbolInfo(const StampProtocol::StampMessage& message);
      void ReadLoop();
  };

  template<typename M, typename S, typename T>
  template<typename MF, typename SF, typename TF>
  TmxIpMarketDataFeedClient<M, S, T>::TmxIpMarketDataFeedClient(
      TmxIpConfiguration config, MF&& marketDataFeedClient,
      SF&& serviceAccessClient, TF&& timeClient)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_config(std::move(config)),
            m_marketDataFeedClient(std::forward<MF>(marketDataFeedClient)),
            m_serviceAccessClient(std::forward<SF>(serviceAccessClient)),
            m_timeClient(std::forward<TF>(timeClient)),
            m_readLoopRoutine(Beam::Routines::Spawn(
              std::bind(&TmxIpMarketDataFeedClient::ReadLoop, this))) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to initialize the TMX IP client."));
  }

  template<typename M, typename S, typename T>
  TmxIpMarketDataFeedClient<M, S, T>::~TmxIpMarketDataFeedClient() {
    Close();
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_timeClient->Close();
    m_serviceAccessClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.Close();
  }

  template<typename M, typename S, typename T>
  std::int64_t TmxIpMarketDataFeedClient<M, S, T>::GetBoardLotPortion(
      std::int64_t quantity, Money price) {
    if(price < 10 * Money::CENT) {
      return quantity - (quantity % 1000);
    } else if(price < 99 * Money::CENT) {
      return quantity - (quantity % 500);
    } else {
      return quantity - (quantity % 100);
    }
  }

  template<typename M, typename S, typename T>
  std::int64_t TmxIpMarketDataFeedClient<M, S, T>::RoundToBoardLotPortion(
      std::int64_t quantity, Money price) {
    if(price < 10 * Money::CENT) {
      return quantity + 1000 - (quantity % 1000);
    } else if(price < 99 * Money::CENT) {
      return quantity + 500 - (quantity % 500);
    } else {
      return quantity + 100 - (quantity % 100);
    }
  }

  template<typename M, typename S, typename T>
  boost::optional<boost::posix_time::ptime> TmxIpMarketDataFeedClient<M, S, T>::
      GetTimestamp(const StampProtocol::StampMessage& message, int index) {
    auto timestamp = message.GetBusinessField<boost::posix_time::ptime>(index);
    if(timestamp) {
      *timestamp += m_config.m_timeOffset;
    }
    return timestamp;
  }

  template<typename M, typename S, typename T>
  const std::string& TmxIpMarketDataFeedClient<M, S, T>::GetMpid(
      const std::string& brokerNumber) const {
    if(brokerNumber.empty()) {
      return m_config.m_defaultMpid;
    }
    auto normalizedBrokerNumber = [&] () -> std::string {
      auto i = brokerNumber.find_first_not_of('0');
      if(i == std::string::npos) {
        return "0";
      }
      return brokerNumber.substr(i);
    }();
    auto i = m_config.m_mpidMappings.find(normalizedBrokerNumber);
    if(i != m_config.m_mpidMappings.end()) {
      return i->second;
    }
    return brokerNumber;
  }

  template<typename M, typename S, typename T>
  std::string TmxIpMarketDataFeedClient<M, S, T>::GetOrderId(
      const boost::optional<std::string>& symbol,
      const boost::optional<std::string>& brokerNumber,
      const std::string& orderNumber) {
    auto result = std::string();
    if(symbol) {
      result = *symbol;
      result += '-';
    }
    if(m_config.m_useBrokerNumberAsKey && brokerNumber) {
      if(brokerNumber->size() == 1) {
        result += '0';
        result += '0';
      } else if(brokerNumber->size() == 2) {
        result += '0';
      }
      result.append(*brokerNumber);
      result += '-';
    }
    result += orderNumber;
    return result;
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::HandleQuote(
      const StampProtocol::StampMessage& message) {
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol) {
      return;
    }
    auto bidPrice = message.GetBusinessField<Money>(196, 0);
    if(!bidPrice) {
      return;
    }
    auto bidVolume = message.GetBusinessField<std::int64_t>(64, 0);
    if(!bidVolume) {
      return;
    }
    auto askPrice = message.GetBusinessField<Money>(196, 1);
    if(!askPrice) {
      return;
    }
    auto askVolume = message.GetBusinessField<std::int64_t>(64, 1);
    if(!askVolume) {
      return;
    }
    if(m_config.m_market == DefaultMarkets::CSE()) {
      auto bidExchangeId = message.GetBusinessField<std::string>(247, 0);
      if(!bidExchangeId || *bidExchangeId != "CNQ") {
        return;
      }
    }
    auto security = Security(std::move(*symbol), m_config.m_market,
      m_config.m_country);
    auto bid = Quote(*bidPrice, *bidVolume, Side::BID);
    auto ask = Quote(*askPrice, *askVolume, Side::ASK);
    auto bbo = BboQuote(bid, ask, m_timeClient->GetTime());
    m_marketDataFeedClient->Publish(SecurityBboQuote(bbo, std::move(security)));
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::HandleLastSaleTradeReport(
      const StampProtocol::StampMessage& message) {
    auto businessAction = message.GetBusinessField<std::string>(5);
    if(!businessAction) {
      return;
    }
    if(*businessAction == "Cancelled") {
      return;
    }
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol) {
      return;
    }
    auto price = message.GetBusinessField<Money>(41);
    if(!price) {
      return;
    }
    auto volume = message.GetBusinessField<std::int64_t>(64);
    if(!volume) {
      return;
    }
    auto exchangeId = message.GetBusinessField<std::string>(247);
    if(!exchangeId) {
      return;
    }
    auto buyerMpid =
      GetMpid(message.GetBusinessField<std::string>(70, 0).value_or(""));
    auto sellerMpid =
      GetMpid(message.GetBusinessField<std::string>(70, 1).value_or(""));
    auto security = Security(std::move(*symbol), m_config.m_market,
      m_config.m_country);
    auto condition = TimeAndSale::Condition();
    condition.m_code = "@";
    auto timeAndSale = TimeAndSale(*timestamp, *price, *volume,
      std::move(condition), *exchangeId, std::move(buyerMpid),
      std::move(sellerMpid));
    m_marketDataFeedClient->Publish(
      SecurityTimeAndSale(timeAndSale, std::move(security)));
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::HandleOrderInfo(
      const StampProtocol::StampMessage& message) {
    HandleBookedOrder(message);
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::HandleBookedOrder(
      const StampProtocol::StampMessage& message) {
    auto nonResidentFlag = message.GetBusinessField<std::string>(168);
    if(nonResidentFlag && *nonResidentFlag == "Y") {
      return;
    }
    auto settlementTerms = message.GetBusinessField<std::string>(53);
    if(settlementTerms) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol) {
      return;
    }
    auto orderNumber = [&] {
      if(!m_config.m_isNeoBook) {
        return message.GetBusinessField<std::string>(40);
      } else {
        auto aqnTag = message.GetBusinessField<std::string>(636);
        if(!aqnTag || *aqnTag != "AQN") {
          return message.GetBusinessField<std::string>(40);
        } else {
          return message.GetBusinessField<std::string>(196);
        }
      }
    }();
    if(!orderNumber) {
      return;
    }
    auto mpid = std::string();
    auto isPrimaryMpid = bool();
    if(m_config.m_consolidateMpids) {
      if(!m_config.m_isNeoBook) {
        mpid = m_config.m_defaultMpid;
        isPrimaryMpid = true;
      } else {
        auto mpidField = message.GetBusinessField<std::string>(636);
        if(!mpidField || mpidField->empty()) {
          mpid = m_config.m_defaultMpid;
          isPrimaryMpid = true;
        } else {
          mpid = *mpidField;
          isPrimaryMpid = false;
        }
        isPrimaryMpid = (mpid == "AQL");
      }
    } else {
      auto mpidField = message.GetBusinessField<std::string>(70);
      if(!mpidField || mpidField->empty()) {
        mpid = m_config.m_defaultMpid;
        isPrimaryMpid = true;
      } else {
        mpid = *mpidField;
        auto mpidNameIterator = m_config.m_mpidMappings.find(mpid);
        if(mpidNameIterator != m_config.m_mpidMappings.end()) {
          mpid = mpidNameIterator->second;
        }
        isPrimaryMpid = false;
      }
    }
    if(mpid.empty()) {
      return;
    }
    auto price = message.GetBusinessField<Money>(196);
    if(!price) {
      return;
    }
    auto quantity = message.GetBusinessField<std::int64_t>(64);
    if(!quantity) {
      return;
    }
    auto side = message.GetBusinessField<Side>(5);
    if(!side) {
      return;
    }
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto brokerNumber = message.GetBusinessField<std::string>(70);
    auto orderId = GetOrderId(symbol, brokerNumber, *orderNumber);
    if(m_config.m_market == DefaultMarkets::OMGA() ||
        m_config.m_market == DefaultMarkets::LYNX()) {
      if(auto modificationId = message.GetBusinessField<std::string>(11)) {
        auto previousId = GetOrderId(symbol, brokerNumber, *modificationId);
        m_marketDataFeedClient->DeleteOrder(previousId, *timestamp);
      }
    } else if(m_config.m_isNeoBook && !isPrimaryMpid) {
      m_marketDataFeedClient->DeleteOrder(orderId, *timestamp);
    }
    auto security = Security(std::move(*symbol), m_config.m_market,
      m_config.m_country);
    *quantity = GetBoardLotPortion(*quantity, *price);
    m_marketDataFeedClient->AddOrder(security, m_config.m_market, mpid,
      isPrimaryMpid, orderId, *side, *price, *quantity, *timestamp);
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::HandleCancelledOrder(
      const StampProtocol::StampMessage& message) {
    auto orderNumber = [&] {
      if(!m_config.m_isNeoBook) {
        return message.GetBusinessField<std::string>(40);
      } else {
        auto aqnTag = message.GetBusinessField<std::string>(636);
        if(!aqnTag || *aqnTag != "AQN") {
          return message.GetBusinessField<std::string>(40);
        } else {
          return message.GetBusinessField<std::string>(196);
        }
      }
    }();
    if(!orderNumber) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    auto brokerNumber = message.GetBusinessField<std::string>(70);
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto orderId = GetOrderId(symbol, brokerNumber, *orderNumber);
    m_marketDataFeedClient->DeleteOrder(orderId, *timestamp);
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::HandlePriceAssignedOrder(
      const StampProtocol::StampMessage& message) {
    auto price = message.GetBusinessField<Money>(196);
    if(!price) {
      return;
    }
    auto orderNumber = message.GetBusinessField<std::string>(40);
    if(!orderNumber) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    auto brokerNumber = message.GetBusinessField<std::string>(70);
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto orderId = GetOrderId(symbol, brokerNumber, *orderNumber);
    m_marketDataFeedClient->ModifyOrderPrice(orderId, *price, *timestamp);
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::
      HandleOrderOrCancelConfirmationReport(
      const StampProtocol::StampMessage& message) {
    auto confirmationType = message.GetBusinessField<std::string>(16);
    if(!confirmationType) {
      return;
    }
    if(*confirmationType == "Booked") {
      HandleBookedOrder(message);
    } else if(*confirmationType == "Cancelled") {
      HandleCancelledOrder(message);
    } else if(*confirmationType == "PriceAssigned") {
      HandlePriceAssignedOrder(message);
    }
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::HandleOrderTradeReport(
      const StampProtocol::StampMessage& message) {
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto volume = message.GetBusinessField<std::int64_t>(64);
    if(!volume) {
      return;
    }
    auto price = message.GetBusinessField<Money>(41);
    if(!price) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    auto bidBrokerNumber = message.GetBusinessField<std::string>(70, 0);
    auto bidOrderNumber = [&] {
      if(!m_config.m_isNeoBook) {
        return message.GetBusinessField<std::string>(40, 0);
      } else {
        auto aqnTag = message.GetBusinessField<std::string>(636);
        if(!aqnTag || *aqnTag != "AQN") {
          return message.GetBusinessField<std::string>(40, 0);
        } else {
          return message.GetBusinessField<std::string>(196);
        }
      }
    }();
    if(bidOrderNumber) {
      auto bidOrderId = GetOrderId(symbol, bidBrokerNumber, *bidOrderNumber);
      auto displayVolume = message.GetBusinessField<std::int64_t>(150, 0);
      if(displayVolume) {
        *displayVolume = GetBoardLotPortion(*displayVolume, *price);
        m_marketDataFeedClient->ModifyOrderSize(bidOrderId, *displayVolume,
          *timestamp);
      } else {
        *volume = RoundToBoardLotPortion(*volume, *price);
        m_marketDataFeedClient->OffsetOrderSize(bidOrderId, -*volume,
          *timestamp);
      }
    }
    auto askBrokerNumber = message.GetBusinessField<std::string>(70, 1);
    auto askOrderNumber = [&] {
      if(!m_config.m_isNeoBook) {
        return message.GetBusinessField<std::string>(40, 1);
      } else {
        auto aqnTag = message.GetBusinessField<std::string>(636);
        if(!aqnTag || *aqnTag != "AQN") {
          return message.GetBusinessField<std::string>(40, 1);
        } else {
          return message.GetBusinessField<std::string>(196);
        }
      }
    }();
    if(askOrderNumber) {
      auto askOrderId = GetOrderId(symbol, askBrokerNumber, *askOrderNumber);
      if(auto displayVolume = message.GetBusinessField<std::int64_t>(150, 1)) {
        *displayVolume = GetBoardLotPortion(*displayVolume, *price);
        m_marketDataFeedClient->ModifyOrderSize(askOrderId, *displayVolume,
          *timestamp);
      } else {
        *volume = RoundToBoardLotPortion(*volume, *price);
        m_marketDataFeedClient->OffsetOrderSize(askOrderId, -*volume,
          *timestamp);
      }
    }
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::HandleImbalanceStatus(
      const StampProtocol::StampMessage& message) {
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto exchangeId = message.GetBusinessField<std::string>(247);
    if(!exchangeId) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol) {
      return;
    }
    auto imbalanceSide = message.GetBusinessField<Side>(492);
    if(!imbalanceSide || *imbalanceSide == Side::NONE) {
      return;
    }
    auto imbalanceVolume = message.GetBusinessField<std::int64_t>(493);
    if(!imbalanceVolume) {
      return;
    }
    auto security = Security(std::move(*symbol), m_config.m_market,
      m_config.m_country);
    auto imbalance = MarketOrderImbalance(OrderImbalance(std::move(security),
      *imbalanceSide, *imbalanceVolume, Money::ZERO, *timestamp),
      m_config.m_market);
    m_marketDataFeedClient->Publish(imbalance);
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::
    HandleMbxAssignCalculatedOpeningPrice(
    const StampProtocol::StampMessage& message) {}

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::HandleMbxAssignLimit(
    const StampProtocol::StampMessage& message) {}

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::HandleMbxMessage(
      const StampProtocol::StampMessage& message) {
    auto businessAction = message.GetBusinessField<std::string>(55);
    if(!businessAction) {
      return;
    }
    if(*businessAction == "AssignCOP") {
      HandleMbxAssignCalculatedOpeningPrice(message);
    } else if(*businessAction == "AssignLimit") {
      HandleMbxAssignLimit(message);
    }
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::HandleSymbolInfo(
      const StampProtocol::StampMessage& message) {
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol) {
      return;
    }
    auto listingMarket = message.GetBusinessField<std::string>(554);
    if(!listingMarket) {
      return;
    }
    auto market = [&] {
      if(*listingMarket == "T") {
        return DefaultMarkets::TSX();
      } else if(*listingMarket == "V") {
        return DefaultMarkets::TSXV();
      } else if(*listingMarket == "N") {
        return DefaultMarkets::CSE();
      } else if(*listingMarket == "O") {
        return DefaultMarkets::OMGA();
      } else if(*listingMarket == "Y") {
        return DefaultMarkets::NYSE();
      } else if(*listingMarket == "Q") {
        return DefaultMarkets::NASDAQ();
      } else if(*listingMarket == "A") {
        return DefaultMarkets::ASEX();
      } else if(*listingMarket == "E") {
        return DefaultMarkets::NEOE();
      }
      return MarketCode();
    }();
    if(market == MarketCode()) {
      return;
    }
    auto security = Security(std::move(*symbol), market, m_config.m_country);
    auto name = message.GetBusinessField<std::string>(177).value_or("");
    auto boardLot = message.GetBusinessField<std::int64_t>(115).value_or(0);
    auto info = SecurityInfo(std::move(security), std::move(name), "",
      boardLot);
    m_marketDataFeedClient->Add(info);
  }

  template<typename M, typename S, typename T>
  void TmxIpMarketDataFeedClient<M, S, T>::ReadLoop() {
    constexpr auto BUSINESS_CLASS_FIELD_ID = 6;
    while(true) {
      auto message = std::optional<StampProtocol::StampMessage>();
      try {
        message.emplace(m_serviceAccessClient->Read());
      } catch(const Beam::IO::EndOfFileException&) {
        break;
      }
      if(m_config.m_isLoggingMessages) {
        std::cout << message->GetHeader().m_sequenceNumber << ": " <<
          std::string(message->GetBusinessContentData(),
          message->GetBusinessContentSize()) << "\n";
      }
      auto businessClass = message->GetBusinessField<std::string>(
        BUSINESS_CLASS_FIELD_ID);
      if(!businessClass) {
        continue;
      }
      if(*businessClass == "OrderCancelResp") {
        HandleOrderOrCancelConfirmationReport(*message);
      } else if(*businessClass == "Quote") {
        HandleQuote(*message);
      } else if(*businessClass == "TradeReport") {
        if(m_config.m_isTimeAndSaleFeed) {
          HandleLastSaleTradeReport(*message);
        } else {
          HandleOrderTradeReport(*message);
        }
      } else if(*businessClass == "OrderInfo") {
        HandleOrderInfo(*message);
      } else if(*businessClass == "MocImbalanceStatus") {
        HandleImbalanceStatus(*message);
      } else if(*businessClass == "MBXMessage") {
        HandleMbxMessage(*message);
      } else if(*businessClass == "SymbolInfo") {
        HandleSymbolInfo(*message);
      }
    }
  }
}

#endif
