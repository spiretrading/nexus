#ifndef NEXUS_CSE_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_CSE_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "CseMarketDataFeedClient/CseConfiguration.hpp"
#include "CseMarketDataFeedClient/CseServiceAccessClient.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class CseMarketDataFeedClient
      \brief Parses packets from the CSE data feed.
      \tparam MarketDataFeedClientType The type of MarketDataFeedClient used to
              update the MarketDataServer.
      \tparam ServiceAccessClientType The type of service access client
              receiving messages.
      \tparam TimeClientType The type of TimeClient used for timestamps.
   */
  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  class CseMarketDataFeedClient : private boost::noncopyable {
    public:

      //! The type of MarketDataFeedClient used to update the MarketDataServer.
      using MarketDataFeedClient = Beam::GetTryDereferenceType<
        MarketDataFeedClientType>;

      //! The type of channel receiving the market data feed.
      using ServiceAccessClient = Beam::GetTryDereferenceType<
        ServiceAccessClientType>;

      //! The type of TimeClient used for timestamps.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! Constructs a CseMarketDataFeedClient.
      /*!
        \param config The configuration to use.
        \param marketDataFeedClient Initializes the MarketDataFeedClient.
        \param serviceAccessClient The service access client receiving messages.
        \param timeClient The TimeClient used for timestamps.
      */
      template<typename MarketDataFeedClientForward,
        typename ServiceAccessClientForward, typename TimeClientForward>
      CseMarketDataFeedClient(const CseConfiguration& config,
        MarketDataFeedClientForward&& marketDataFeedClient,
        ServiceAccessClientForward&& serviceAccessClient,
        TimeClientForward&& timeClient);

      ~CseMarketDataFeedClient();

      void Open();

      void Close();

    private:
      CseConfiguration m_config;
      Beam::GetOptionalLocalPtr<MarketDataFeedClientType>
        m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<ServiceAccessClientType> m_serviceAccessClient;
      Beam::GetOptionalLocalPtr<TimeClientType> m_timeClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      static Quantity GetBoardLotPortion(Quantity quantity, Money price);
      static Quantity RoundToBoardLotPortion(Quantity quantity, Money price);
      void Shutdown();
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
      void HandleImbalanceStatus(const StampProtocol::StampMessage& message);
      void ReadLoop();
  };

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  template<typename MarketDataFeedClientForward,
    typename ServiceAccessClientForward, typename TimeClientForward>
  CseMarketDataFeedClient<MarketDataFeedClientType, ServiceAccessClientType,
      TimeClientType>::CseMarketDataFeedClient(
      const CseConfiguration& config,
      MarketDataFeedClientForward&& marketDataFeedClient,
      ServiceAccessClientForward&& serviceAccessClient,
      TimeClientForward&& timeClient)
      : m_config(config),
        m_marketDataFeedClient(std::forward<MarketDataFeedClientForward>(
          marketDataFeedClient)),
        m_serviceAccessClient(std::forward<ServiceAccessClientForward>(
          serviceAccessClient)),
        m_timeClient(std::forward<TimeClientForward>(timeClient)) {}

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  CseMarketDataFeedClient<MarketDataFeedClientType, ServiceAccessClientType,
      TimeClientType>::~CseMarketDataFeedClient() {
    Close();
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  void CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataFeedClient->Open();
      m_serviceAccessClient->Open();
      m_timeClient->Open();
      m_readLoopRoutine = Beam::Routines::Spawn(
        std::bind(&CseMarketDataFeedClient::ReadLoop, this));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  void CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  Quantity CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::GetBoardLotPortion(
      Quantity quantity, Money price) {
    if(price < 10 * Money::CENT) {
      return quantity - (quantity % 1000);
    } else if(price < 99 * Money::CENT) {
      return quantity - (quantity % 500);
    } else {
      return quantity - (quantity % 100);
    }
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  Quantity CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::RoundToBoardLotPortion(
      Quantity quantity, Money price) {
    if(price < 10 * Money::CENT) {
      return quantity + 1000 - (quantity % 1000);
    } else if(price < 99 * Money::CENT) {
      return quantity + 500 - (quantity % 500);
    } else {
      return quantity + 100 - (quantity % 100);
    }
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  void CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::Shutdown() {
    m_timeClient->Close();
    m_serviceAccessClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.SetClosed();
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  boost::optional<boost::posix_time::ptime> CseMarketDataFeedClient<
      MarketDataFeedClientType, ServiceAccessClientType, TimeClientType>::
      GetTimestamp(const StampProtocol::StampMessage& message, int index) {
    auto timestamp = message.GetBusinessField<boost::posix_time::ptime>(index);
    if(timestamp.is_initialized()) {
      *timestamp += m_config.m_timeOffset;
    }
    return timestamp;
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  std::string CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::GetOrderId(
      const boost::optional<std::string>& symbol,
      const boost::optional<std::string>& brokerNumber,
      const std::string& orderNumber) {
    std::string result;
    if(symbol.is_initialized()) {
      result = *symbol;
      result += '-';
    }
    if(brokerNumber.is_initialized()) {
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

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  void CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::HandleQuote(
      const StampProtocol::StampMessage& message) {
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol.is_initialized() ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto bidPrice = message.GetBusinessField<Money>(196, 0);
    if(!bidPrice.is_initialized()) {
      return;
    }
    auto bidVolume = message.GetBusinessField<Quantity>(64, 0);
    if(!bidVolume.is_initialized()) {
      return;
    }
    auto askPrice = message.GetBusinessField<Money>(196, 1);
    if(!askPrice.is_initialized()) {
      return;
    }
    auto askVolume = message.GetBusinessField<Quantity>(64, 1);
    if(!askVolume.is_initialized()) {
      return;
    }
    Security security(std::move(*symbol), DefaultMarkets::CSE(),
      DefaultCountries::CA());
    Quote bid(*bidPrice, *bidVolume, Side::BID);
    Quote ask(*askPrice, *askVolume, Side::ASK);
    BboQuote bbo(bid, ask, m_timeClient->GetTime());
    m_marketDataFeedClient->PublishBboQuote(SecurityBboQuote(bbo,
      std::move(security)));
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  void CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::HandleLastSaleTradeReport(
      const StampProtocol::StampMessage& message) {
    auto businessAction = message.GetBusinessField<std::string>(5);
    if(!businessAction.is_initialized()) {
      return;
    }
    if(*businessAction == "Cancelled") {
      return;
    }
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp.is_initialized()) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol.is_initialized() ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto price = message.GetBusinessField<Money>(41);
    if(!price.is_initialized()) {
      return;
    }
    auto volume = message.GetBusinessField<Quantity>(64);
    if(!volume.is_initialized()) {
      return;
    }
    auto exchangeId = message.GetBusinessField<std::string>(247);
    if(!exchangeId.is_initialized()) {
      return;
    }
    Security security(std::move(*symbol), DefaultMarkets::CSE(),
      DefaultCountries::CA());
    TimeAndSale::Condition condition;
    condition.m_code = "@";
    TimeAndSale timeAndSale(*timestamp, *price, *volume, std::move(condition),
      *exchangeId);
    m_marketDataFeedClient->PublishTimeAndSale(
      SecurityTimeAndSale(timeAndSale, std::move(security)));
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  void CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::HandleOrderInfo(
      const StampProtocol::StampMessage& message) {
    HandleBookedOrder(message);
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  void CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::HandleBookedOrder(
      const StampProtocol::StampMessage& message) {
    auto nonResidentFlag = message.GetBusinessField<std::string>(168);
    if(nonResidentFlag.is_initialized() && *nonResidentFlag == "Y") {
      return;
    }
    auto settlementTerms = message.GetBusinessField<std::string>(53);
    if(settlementTerms.is_initialized()) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol.is_initialized() ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto orderNumber = message.GetBusinessField<std::string>(40);
    if(!orderNumber.is_initialized()) {
      return;
    }
    auto mpidField = message.GetBusinessField<std::string>(70);
    if(!mpidField.is_initialized() || mpidField->empty()) {
      return;
    }
    auto mpid = *mpidField;
    auto mpidNameIterator = m_config.m_mpidMappings.find(mpid);
    if(mpidNameIterator != m_config.m_mpidMappings.end()) {
      mpid = mpidNameIterator->second;
    }
    auto price = message.GetBusinessField<Money>(196);
    if(!price.is_initialized()) {
      return;
    }
    auto quantity = message.GetBusinessField<Quantity>(64);
    if(!quantity.is_initialized()) {
      return;
    }
    auto side = message.GetBusinessField<Side>(5);
    if(!side.is_initialized()) {
      return;
    }
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp.is_initialized()) {
      return;
    }
    auto brokerNumber = message.GetBusinessField<std::string>(70);
    auto orderId = GetOrderId(symbol, brokerNumber, *orderNumber);
    Security security(std::move(*symbol), DefaultMarkets::CSE(),
      DefaultCountries::CA());
    *quantity = GetBoardLotPortion(*quantity, *price);
    m_marketDataFeedClient->AddOrder(security, DefaultMarkets::CSE(), mpid,
      false, orderId, *side, *price, *quantity, *timestamp);
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  void CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::HandleCancelledOrder(
      const StampProtocol::StampMessage& message) {
    auto orderNumber = message.GetBusinessField<std::string>(40);
    if(!orderNumber.is_initialized()) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol.is_initialized() ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto brokerNumber = message.GetBusinessField<std::string>(70);
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp.is_initialized()) {
      return;
    }
    auto orderId = GetOrderId(symbol, brokerNumber, *orderNumber);
    m_marketDataFeedClient->DeleteOrder(orderId, *timestamp);
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  void CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::HandlePriceAssignedOrder(
      const StampProtocol::StampMessage& message) {
    auto price = message.GetBusinessField<Money>(196);
    if(!price.is_initialized()) {
      return;
    }
    auto orderNumber = message.GetBusinessField<std::string>(40);
    if(!orderNumber.is_initialized()) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol.is_initialized() ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto brokerNumber = message.GetBusinessField<std::string>(70);
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp.is_initialized()) {
      return;
    }
    auto orderId = GetOrderId(symbol, brokerNumber, *orderNumber);
    m_marketDataFeedClient->ModifyOrderPrice(orderId, *price, *timestamp);
  }

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  void CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::
      HandleOrderOrCancelConfirmationReport(
      const StampProtocol::StampMessage& message) {
    auto confirmationType = message.GetBusinessField<std::string>(16);
    if(!confirmationType.is_initialized()) {
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

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  void CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::HandleOrderTradeReport(
      const StampProtocol::StampMessage& message) {
    auto timestamp = GetTimestamp(message, 57);
    if(!timestamp.is_initialized()) {
      return;
    }
    auto volume = message.GetBusinessField<Quantity>(64);
    if(!volume.is_initialized()) {
      return;
    }
    auto price = message.GetBusinessField<Money>(41);
    if(!price.is_initialized()) {
      return;
    }
    auto symbol = message.GetBusinessField<std::string>(55);
    if(!symbol.is_initialized() ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto bidBrokerNumber = message.GetBusinessField<std::string>(70, 0);
    auto bidOrderNumber = message.GetBusinessField<std::string>(40, 0);
    if(bidOrderNumber.is_initialized()) {
      auto bidOrderId = GetOrderId(symbol, bidBrokerNumber, *bidOrderNumber);
      auto displayVolume = message.GetBusinessField<Quantity>(150, 0);
      if(displayVolume.is_initialized()) {
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
    if(askOrderNumber.is_initialized()) {
      auto askOrderId = GetOrderId(symbol, askBrokerNumber, *askOrderNumber);
      auto displayVolume = message.GetBusinessField<Quantity>(150, 1);
      if(displayVolume.is_initialized()) {
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

  template<typename MarketDataFeedClientType, typename ServiceAccessClientType,
    typename TimeClientType>
  void CseMarketDataFeedClient<MarketDataFeedClientType,
      ServiceAccessClientType, TimeClientType>::ReadLoop() {
    static const auto BUSINESS_CLASS_FIELD_ID = 6;
    while(true) {
      Beam::DelayPtr<StampProtocol::StampMessage> message;
      try {
        message.Initialize(m_serviceAccessClient->Read());
      } catch(const Beam::IO::NotConnectedException&) {
        break;
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
}

#endif
