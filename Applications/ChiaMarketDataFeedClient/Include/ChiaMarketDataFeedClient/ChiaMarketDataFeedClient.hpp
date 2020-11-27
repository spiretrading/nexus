#ifndef NEXUS_CHIA_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_CHIA_MARKET_DATA_FEED_CLIENT_HPP
#include <cstdint>
#include <string>
#include <unordered_map>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "ChiaMarketDataFeedClient/ChiaConfiguration.hpp"
#include "ChiaMarketDataFeedClient/ChiaMessage.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolClient.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolMessage.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /**
   * Parses packets from the CHIA market data feed.
   * @param <M> The type of MarketDataFeedClient used to update the
   *        MarketDataServer.
   * @param <P> The type of client receiving CHIA messages.
   */
  template<typename M, typename P>
  class ChiaMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::GetTryDereferenceType<M>;

      /** The type of client receiving CHIA messages. */
      using ProtocolClient = Beam::GetTryDereferenceType<P>;

      /**
       * Constructs a ChiaMarketDataFeedClient.
       * @param config The configuration to use.
       * @param marketDataFeedClient Initializes the MarketDataFeedClient.
       * @param protocolClient The client receiving CHIA messages.
       */
      template<typename MF, typename PF>
      ChiaMarketDataFeedClient(ChiaConfiguration config,
        MF&& marketDataFeedClient, PF&& itchClient);

      ~ChiaMarketDataFeedClient();

      void Close();

    private:
      struct OrderEntry {
        Security m_security;
        Money m_price;

        OrderEntry() = default;
        OrderEntry(Security security, Money price);
      };
      ChiaConfiguration m_config;
      Beam::GetOptionalLocalPtr<M> m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<P> m_protocolClient;
      std::unordered_map<std::string, OrderEntry> m_orderEntries;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      ChiaMarketDataFeedClient(const ChiaMarketDataFeedClient&) = delete;
      ChiaMarketDataFeedClient& operator =(
        const ChiaMarketDataFeedClient&) = delete;
      boost::posix_time::ptime ParseTimestamp(
        boost::posix_time::time_duration timestamp);
      void HandleAddOrderMessage(bool isLongForm, const ChiaMessage& message);
      void HandleOrderExecutionMessage(bool isLongForm,
        const ChiaMessage& message);
      void HandleOrderCancelMessage(bool isLongForm,
        const ChiaMessage& message);
      void HandleTradeMessage(bool isLongForm, const ChiaMessage& message);
      void Dispatch(const ChiaMessage& message);
      void ReadLoop();
  };

  template<typename M, typename P>
  ChiaMarketDataFeedClient<M, P>::OrderEntry::OrderEntry(Security security,
    Money price)
    : m_security(std::move(security)),
      m_price(price) {}

  template<typename M, typename P>
  template<typename MF, typename PF>
  ChiaMarketDataFeedClient<M, P>::ChiaMarketDataFeedClient(
      ChiaConfiguration config, MF&& marketDataFeedClient, PF&& protocolClient)
      try : m_config(std::move(config)),
            m_marketDataFeedClient(std::forward<MF>(marketDataFeedClient)),
            m_protocolClient(std::forward<PF>(protocolClient)),
            m_readLoopRoutine(Beam::Routines::Spawn(
              std::bind(&ChiaMarketDataFeedClient::ReadLoop, this))) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Unable to initialize the CHIA market data feed client."));
  }

  template<typename M, typename P>
  ChiaMarketDataFeedClient<M, P>::~ChiaMarketDataFeedClient() {
    Close();
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_protocolClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.Close();
  }

  template<typename M, typename P>
  boost::posix_time::ptime ChiaMarketDataFeedClient<M, P>::ParseTimestamp(
      boost::posix_time::time_duration timestamp) {
    return m_config.m_timeOrigin + timestamp;
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::HandleAddOrderMessage(bool isLongForm,
      const ChiaMessage& message) {
    auto cursor = message.m_data;
    auto orderReference = boost::lexical_cast<std::string>(
      ChiaMessage::ParseNumeric(9, Beam::Store(cursor)));
    auto side = ChiaMessage::ParseSide(Beam::Store(cursor));
    auto shares = [&] {
      if(isLongForm) {
        return ChiaMessage::ParseNumeric(10, Beam::Store(cursor));
      } else {
        return ChiaMessage::ParseNumeric(6, Beam::Store(cursor));
      }
    }();
    auto symbol = ChiaMessage::ParseAlphanumeric(6, Beam::Store(cursor));
    auto price = ChiaMessage::ParsePrice(isLongForm, Beam::Store(cursor));
    auto display = ChiaMessage::ParseChar(Beam::Store(cursor));
    if(display != 'Y') {
      return;
    }
    auto timestamp = ParseTimestamp(message.m_timestamp);
    auto security = Security(symbol, m_config.m_primaryMarket,
      m_config.m_country);
    m_marketDataFeedClient->AddOrder(security, m_config.m_disseminatingMarket,
      m_config.m_mpid, false, orderReference, side, price, shares, timestamp);
    if(m_config.m_isTimeAndSaleFeed) {
      m_orderEntries[orderReference] = OrderEntry(security, price);
    }
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << ',' << message.m_type << ',' <<
        orderReference << ',' << side << ',' << shares << ',' <<
        symbol << ',' << price << '\n';
      std::cout << std::flush;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::HandleOrderExecutionMessage(
      bool isLongForm, const ChiaMessage& message) {
    auto cursor = message.m_data;
    auto orderReference = boost::lexical_cast<std::string>(
      ChiaMessage::ParseNumeric(9, Beam::Store(cursor)));
    auto shares = [&] {
      if(isLongForm) {
        return ChiaMessage::ParseNumeric(10, Beam::Store(cursor));
      } else {
        return ChiaMessage::ParseNumeric(6, Beam::Store(cursor));
      }
    }();
    auto tradeReference = ChiaMessage::ParseNumeric(9, Beam::Store(cursor));
    auto contraOrderReference = boost::lexical_cast<std::string>(
      ChiaMessage::ParseNumeric(9, Beam::Store(cursor)));
    auto timestamp = ParseTimestamp(message.m_timestamp);
    m_marketDataFeedClient->OffsetOrderSize(orderReference, -shares, timestamp);
    if(m_config.m_isTimeAndSaleFeed) {
      auto orderEntry = Beam::Lookup(m_orderEntries, orderReference);
      if(!orderEntry.is_initialized()) {
        return;
      }
      auto condition = TimeAndSale::Condition();
      condition.m_code = "@";
      auto timeAndSale = TimeAndSale(timestamp, orderEntry->m_price, shares,
        std::move(condition), m_config.m_mpid);
      m_marketDataFeedClient->PublishTimeAndSale(
        SecurityTimeAndSale(std::move(timeAndSale), orderEntry->m_security));
    }
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << "," << message.m_type << "," <<
        orderReference << "," << shares << "," << tradeReference << "," <<
        contraOrderReference << "\n";
      std::cout << std::flush;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::HandleOrderCancelMessage(bool isLongForm,
      const ChiaMessage& message) {
    auto cursor = message.m_data;
    auto orderReference = boost::lexical_cast<std::string>(
      ChiaMessage::ParseNumeric(9, Beam::Store(cursor)));
    auto timestamp = ParseTimestamp(message.m_timestamp);
    m_marketDataFeedClient->DeleteOrder(orderReference, timestamp);
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << "," << message.m_type << "," <<
        orderReference << "\n";
      std::cout << std::flush;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::HandleTradeMessage(bool isLongForm,
      const ChiaMessage& message) {
    auto cursor = message.m_data;
    auto orderReference = boost::lexical_cast<std::string>(
      ChiaMessage::ParseNumeric(9, Beam::Store(cursor)));
    auto sideIndicator = ChiaMessage::ParseChar(Beam::Store(cursor));
    auto shares = [&] {
      if(isLongForm) {
        return ChiaMessage::ParseNumeric(10, Beam::Store(cursor));
      } else {
        return ChiaMessage::ParseNumeric(6, Beam::Store(cursor));
      }
    }();
    auto symbol = ChiaMessage::ParseAlphanumeric(6, Beam::Store(cursor));
    auto price = ChiaMessage::ParsePrice(isLongForm, Beam::Store(cursor));
    auto tradeReference = boost::lexical_cast<std::string>(
      ChiaMessage::ParseNumeric(9, Beam::Store(cursor)));
    auto contraOrderReference = boost::lexical_cast<std::string>(
      ChiaMessage::ParseNumeric(9, Beam::Store(cursor)));
    auto tradeType = ChiaMessage::ParseChar(Beam::Store(cursor));
    auto timestamp = ParseTimestamp(message.m_timestamp);
    auto security = Security(symbol, m_config.m_primaryMarket,
      m_config.m_country);
    auto condition = TimeAndSale::Condition();
    condition.m_code = "@";
    auto timeAndSale = TimeAndSale(timestamp, price, shares,
      std::move(condition), m_config.m_mpid);
    m_marketDataFeedClient->PublishTimeAndSale(
      SecurityTimeAndSale(std::move(timeAndSale), security));
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << "," << message.m_type << "," <<
        orderReference << "," << shares << "," << symbol << "," <<
        tradeReference << "," << contraOrderReference << "\n";
      std::cout << std::flush;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::Dispatch(const ChiaMessage& message) {
    if(message.m_type == 'A') {
      HandleAddOrderMessage(false, message);
    } else if(message.m_type == 'a') {
      HandleAddOrderMessage(true, message);
    } else if(message.m_type == 'E') {
      HandleOrderExecutionMessage(false, message);
    } else if(message.m_type == 'e') {
      HandleOrderExecutionMessage(true, message);
    } else if(message.m_type == 'X') {
      HandleOrderCancelMessage(false, message);
    } else if(message.m_type == 'x') {
      HandleOrderCancelMessage(true, message);
    } else if(message.m_type == 'P' || message.m_type == 'M') {
      HandleTradeMessage(false, message);
    } else if(message.m_type == 'p' || message.m_type == 'm') {
      HandleTradeMessage(true, message);
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::ReadLoop() {
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
