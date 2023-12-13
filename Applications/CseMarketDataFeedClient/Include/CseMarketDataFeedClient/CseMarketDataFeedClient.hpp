#ifndef NEXUS_CSE_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_CSE_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "CseMarketDataFeedClient/CseConfiguration.hpp"
#include "CseMarketDataFeedClient/CseServiceAccessClient.hpp"

namespace Nexus::MarketDataService {

  /**
   * Parses packets from the CSE data feed.
   * @param <M> The type of MarketDataFeedClient used to update the
   *        MarketDataServer.
   * @param <S> The type of service access client receiving messages.
   * @param <T> The type of TimeClient used for timestamps.
   */
  template<typename M, typename S, typename T>
  class CseMarketDataFeedClient {
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
       * Constructs a CseMarketDataFeedClient.
       * @param config The configuration to use.
       * @param marketDataFeedClient Initializes the MarketDataFeedClient.
       * @param serviceAccessClient The service access client receiving
       *        messages.
       * @param timeClient The TimeClient used for timestamps.
       */
      template<typename MF, typename SF, typename TF>
      CseMarketDataFeedClient(CseConfiguration config,
        MF&& marketDataFeedClient, SF&& serviceAccessClient, TF&& timeClient);

      ~CseMarketDataFeedClient();

      void Close();

    private:
      CseConfiguration m_config;
      Beam::GetOptionalLocalPtr<M> m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<S> m_serviceAccessClient;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      CseMarketDataFeedClient(const CseMarketDataFeedClient&) = delete;
      CseMarketDataFeedClient& operator =(
        const CseMarketDataFeedClient&) = delete;
      static Quantity GetBoardLotPortion(Quantity quantity, Money price);
      static Quantity RoundToBoardLotPortion(Quantity quantity, Money price);
      boost::optional<boost::posix_time::ptime> GetTimestamp(
        const StampProtocol::StampMessage& message, int index);
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
      void ReadLoop();
  };

  template<typename M, typename S, typename T>
  template<typename MF, typename SF, typename TF>
  CseMarketDataFeedClient<M, S, T>::CseMarketDataFeedClient(
      CseConfiguration config, MF&& marketDataFeedClient,
      SF&& serviceAccessClient, TF&& timeClient)
      try : m_config(std::move(config)),
            m_marketDataFeedClient(std::forward<MF>(marketDataFeedClient)),
            m_serviceAccessClient(std::forward<SF>(serviceAccessClient)),
            m_timeClient(std::forward<TF>(timeClient)),
            m_readLoopRoutine(Beam::Routines::Spawn(
              std::bind(&CseMarketDataFeedClient::ReadLoop, this))) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to initialize the CSE market data feed client."));
  }

  template<typename M, typename S, typename T>
  CseMarketDataFeedClient<M, S, T>::~CseMarketDataFeedClient() {
    Close();
  }

  template<typename M, typename S, typename T>
  void CseMarketDataFeedClient<M, S, T>::Close() {
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
  Quantity CseMarketDataFeedClient<M, S, T>::GetBoardLotPortion(
      Quantity quantity, Money price) {
    if(price < 10 * Money::CENT) {
      return quantity - (quantity % 1000);
    } else if(price < 99 * Money::CENT) {
      return quantity - (quantity % 500);
    } else {
      return quantity - (quantity % 100);
    }
  }

  template<typename M, typename S, typename T>
  Quantity CseMarketDataFeedClient<M, S, T>::RoundToBoardLotPortion(
      Quantity quantity, Money price) {
    if(price < 10 * Money::CENT) {
      return quantity + 1000 - (quantity % 1000);
    } else if(price < 99 * Money::CENT) {
      return quantity + 500 - (quantity % 500);
    } else {
      return quantity + 100 - (quantity % 100);
    }
  }

  template<typename M, typename S, typename T>
  boost::optional<boost::posix_time::ptime> CseMarketDataFeedClient<M, S, T>::
      GetTimestamp(const StampProtocol::StampMessage& message, int index) {
    auto timestamp = message.GetBusinessField<boost::posix_time::ptime>(index);
    if(timestamp) {
      *timestamp += m_config.m_timeOffset;
    }
    return timestamp;
  }

  template<typename M, typename S, typename T>
  std::string CseMarketDataFeedClient<M, S, T>::GetOrderId(
      const boost::optional<std::string>& symbol,
      const boost::optional<std::string>& brokerNumber,
      const std::string& orderNumber) {
    auto result = std::string();
    if(symbol) {
      result = *symbol;
      result += '-';
    }
    if(brokerNumber) {
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
  void CseMarketDataFeedClient<M, S, T>::HandleQuote(
      const StampProtocol::StampMessage& message) {
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto bidPrice = message.GetBusinessField<Money>(196, 0);
    if(!bidPrice || *bidPrice == Money::ZERO) {
      return;
    }
    auto bidVolume = message.GetBusinessField<Quantity>(64, 0);
    if(!bidVolume) {
      return;
    }
    auto askPrice = message.GetBusinessField<Money>(196, 1);
    if(!askPrice || *askPrice == Money::ZERO) {
      return;
    }
    auto askVolume = message.GetBusinessField<Quantity>(64, 1);
    if(!askVolume) {
      return;
    }
    auto security =
      Security(std::move(*symbol), m_config.m_market, DefaultCountries::CA());
    auto bid = Quote(*bidPrice, *bidVolume, Side::BID);
    auto ask = Quote(*askPrice, *askVolume, Side::ASK);
    auto bbo = BboQuote(bid, ask, m_timeClient->GetTime());
    m_marketDataFeedClient->Publish(SecurityBboQuote(bbo, std::move(security)));
  }

  template<typename M, typename S, typename T>
  void CseMarketDataFeedClient<M, S, T>::HandleLastSaleTradeReport(
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
    if(!symbol ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto price = message.GetBusinessField<Money>(41);
    if(!price) {
      return;
    }
    auto volume = message.GetBusinessField<Quantity>(64);
    if(!volume) {
      return;
    }
    auto exchangeId = message.GetBusinessField<std::string>(247);
    if(!exchangeId) {
      return;
    }
    auto security =
      Security(std::move(*symbol), m_config.m_market, DefaultCountries::CA());
    auto condition = TimeAndSale::Condition();
    condition.m_code = "@";
    auto timeAndSale = TimeAndSale(*timestamp, *price, *volume,
      std::move(condition), *exchangeId);
    m_marketDataFeedClient->Publish(SecurityTimeAndSale(
      timeAndSale, std::move(security)));
  }

  template<typename M, typename S, typename T>
  void CseMarketDataFeedClient<M, S, T>::HandleOrderInfo(
      const StampProtocol::StampMessage& message) {
    HandleBookedOrder(message);
  }

  template<typename M, typename S, typename T>
  void CseMarketDataFeedClient<M, S, T>::HandleBookedOrder(
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
    if(!symbol ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto orderNumber = message.GetBusinessField<std::string>(40);
    if(!orderNumber) {
      return;
    }
    auto mpidField = message.GetBusinessField<std::string>(70);
    if(!mpidField || mpidField->empty()) {
      return;
    }
    auto mpid = *mpidField;
    auto mpidNameIterator = m_config.m_mpidMappings.find(mpid);
    if(mpidNameIterator != m_config.m_mpidMappings.end()) {
      mpid = mpidNameIterator->second;
    }
    auto price = message.GetBusinessField<Money>(196);
    if(!price) {
      return;
    }
    auto quantity = message.GetBusinessField<Quantity>(64);
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
    auto security =
      Security(std::move(*symbol), m_config.m_market, DefaultCountries::CA());
    *quantity = GetBoardLotPortion(*quantity, *price);
    m_marketDataFeedClient->AddOrder(security, m_config.m_market, mpid, false,
      orderId, *side, *price, *quantity, *timestamp);
  }

  template<typename M, typename S, typename T>
  void CseMarketDataFeedClient<M, S, T>::HandleCancelledOrder(
      const StampProtocol::StampMessage& message) {
    auto orderNumber = message.GetBusinessField<std::string>(40);
    if(!orderNumber) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto brokerNumber = message.GetBusinessField<std::string>(70);
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto orderId = GetOrderId(symbol, brokerNumber, *orderNumber);
    m_marketDataFeedClient->DeleteOrder(orderId, *timestamp);
  }

  template<typename M, typename S, typename T>
  void CseMarketDataFeedClient<M, S, T>::HandlePriceAssignedOrder(
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
    if(!symbol ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto brokerNumber = message.GetBusinessField<std::string>(70);
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto orderId = GetOrderId(symbol, brokerNumber, *orderNumber);
    m_marketDataFeedClient->ModifyOrderPrice(orderId, *price, *timestamp);
  }

  template<typename M, typename S, typename T>
  void CseMarketDataFeedClient<M, S, T>::HandleOrderOrCancelConfirmationReport(
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
  void CseMarketDataFeedClient<M, S, T>::HandleOrderTradeReport(
      const StampProtocol::StampMessage& message) {
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto volume = message.GetBusinessField<Quantity>(64);
    if(!volume) {
      return;
    }
    auto price = message.GetBusinessField<Money>(41);
    if(!price) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto bidBrokerNumber = message.GetBusinessField<std::string>(70, 0);
    auto bidOrderNumber = message.GetBusinessField<std::string>(40, 0);
    if(bidOrderNumber) {
      auto bidOrderId = GetOrderId(symbol, bidBrokerNumber, *bidOrderNumber);
      auto displayVolume = message.GetBusinessField<Quantity>(150, 0);
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
    auto askOrderNumber = message.GetBusinessField<std::string>(40, 1);
    if(askOrderNumber) {
      auto askOrderId = GetOrderId(symbol, askBrokerNumber, *askOrderNumber);
      auto displayVolume = message.GetBusinessField<Quantity>(150, 1);
      if(displayVolume) {
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
  void CseMarketDataFeedClient<M, S, T>::ReadLoop() {
    static constexpr auto BUSINESS_CLASS_FIELD_ID = 6;
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
      if(!businessClass.is_initialized()) {
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
      }
    }
  }
}

#endif
