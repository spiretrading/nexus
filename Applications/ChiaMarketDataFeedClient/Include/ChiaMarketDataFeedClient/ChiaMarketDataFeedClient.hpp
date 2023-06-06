#ifndef NEXUS_CHIA_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_CHIA_MARKET_DATA_FEED_CLIENT_HPP
#include <string>
#include <unordered_map>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include "ChiaMarketDataFeedClient/ChiaConfiguration.hpp"
#include "ChiaMarketDataFeedClient/PitchMessage.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /**
   * Parses PITCH messages from the CHIA market data feed.
   * @param <M> The type of MarketDataFeedClient used to update the
   *        MarketDataServer.
   * @param <P> The type of client receiving PITCH messages.
   */
  template<typename M, typename P>
  class ChiaMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::GetTryDereferenceType<M>;

      /** The type of client receiving PITCH messages. */
      using ProtocolClient = Beam::GetTryDereferenceType<P>;

      /**
       * Constructs a ChiaMarketDataFeedClient.
       * @param config The configuration to use.
       * @param marketDataFeedClient Initializes the MarketDataFeedClient.
       * @param protocolClient The client receiving PITCH messages.
       */
      template<typename MF, typename PF>
      ChiaMarketDataFeedClient(
        ChiaConfiguration config, MF&& marketDataFeedClient, PF&& itchClient);

      ~ChiaMarketDataFeedClient();

      void Close();

    private:
      struct OrderEntry {
        Security m_security;
        Money m_price;
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
      void HandleAddOrderMessage(const PitchMessage& message);
      void HandleOrderExecutedMessage(const PitchMessage& message);
      void HandleReduceSizeMessage(const PitchMessage& message);
      void HandleModifyOrderMessage(const PitchMessage& message);
      void HandleDeleteOrderMessage(const PitchMessage& message);
      void HandleTradeMessage(const PitchMessage& message);
      void Dispatch(const PitchMessage& message);
      void ReadLoop();
  };

  template<typename M, typename P>
  template<typename MF, typename PF>
  ChiaMarketDataFeedClient<M, P>::ChiaMarketDataFeedClient(
      ChiaConfiguration config, MF&& marketDataFeedClient, PF&& protocolClient)
      try : m_config(std::move(config)),
            m_marketDataFeedClient(std::forward<MF>(marketDataFeedClient)),
            m_protocolClient(std::forward<PF>(protocolClient)),
            m_readLoopRoutine(Beam::Routines::Spawn(
              std::bind_front(&ChiaMarketDataFeedClient::ReadLoop, this))) {
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
  void ChiaMarketDataFeedClient<M, P>::HandleAddOrderMessage(
      const PitchMessage& message) {
    auto cursor = message.m_payload;
    auto timestamp = PitchMessage::ParseTimestamp(Beam::Store(cursor));
    auto orderId =
      std::to_string(PitchMessage::ParseUint64(Beam::Store(cursor)));
    auto side = PitchMessage::ParseSide(Beam::Store(cursor));
    auto quantity = PitchMessage::ParseUint32(Beam::Store(cursor));
    if(quantity == 0) {
      return;
    }
    auto symbol = PitchMessage::ParseAlphanumeric(6, Beam::Store(cursor));
    auto price = PitchMessage::ParsePrice(Beam::Store(cursor));
    auto security =
      Security(symbol, m_config.m_primaryMarket, m_config.m_country);
    if(m_config.m_isTimeAndSaleFeed) {
      m_orderEntries[orderId] = OrderEntry(security, price);
    }
    m_marketDataFeedClient->AddOrder(security, m_config.m_disseminatingMarket,
      m_config.m_mpid, false, orderId, side, price, quantity, timestamp);
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << ',' << message.m_type << ',' << orderId <<
        ',' << side << ',' << quantity << ',' << symbol << ',' << price <<
        std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::HandleOrderExecutedMessage(
      const PitchMessage& message) {
    auto cursor = message.m_payload;
    auto timestamp = PitchMessage::ParseTimestamp(Beam::Store(cursor));
    auto orderId =
      std::to_string(PitchMessage::ParseUint64(Beam::Store(cursor)));
    auto executedQuantity = PitchMessage::ParseUint32(Beam::Store(cursor));
    if(executedQuantity == 0) {
      return;
    }
    m_marketDataFeedClient->OffsetOrderSize(
      orderId, -static_cast<std::int32_t>(executedQuantity), timestamp);
    if(m_config.m_isTimeAndSaleFeed) {
      if(auto orderEntry = Beam::Lookup(m_orderEntries, orderId)) {
        auto condition = TimeAndSale::Condition();
        condition.m_code = "@";
        auto timeAndSale = TimeAndSale(timestamp, orderEntry->m_price,
          executedQuantity, std::move(condition), m_config.m_mpid);
        m_marketDataFeedClient->Publish(SecurityTimeAndSale(
          std::move(timeAndSale), orderEntry->m_security));
      }
    }
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << ',' << message.m_type << ',' << orderId <<
        ',' << executedQuantity << std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::HandleReduceSizeMessage(
      const PitchMessage& message) {
    auto cursor = message.m_payload;
    auto timestamp = PitchMessage::ParseTimestamp(Beam::Store(cursor));
    auto orderId =
      std::to_string(PitchMessage::ParseUint64(Beam::Store(cursor)));
    auto cancelledQuantity = PitchMessage::ParseUint32(Beam::Store(cursor));
    if(cancelledQuantity == 0) {
      return;
    }
    m_marketDataFeedClient->OffsetOrderSize(
      orderId, -static_cast<std::int32_t>(cancelledQuantity), timestamp);
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << ',' << message.m_type << ',' << orderId <<
        ',' << cancelledQuantity << std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::HandleModifyOrderMessage(
      const PitchMessage& message) {
    auto cursor = message.m_payload;
    auto timestamp = PitchMessage::ParseTimestamp(Beam::Store(cursor));
    auto orderId =
      std::to_string(PitchMessage::ParseUint64(Beam::Store(cursor)));
    auto quantity = PitchMessage::ParseUint32(Beam::Store(cursor));
    auto price = PitchMessage::ParsePrice(Beam::Store(cursor));
    m_marketDataFeedClient->ModifyOrderSize(orderId, quantity, timestamp);
    m_marketDataFeedClient->ModifyOrderPrice(orderId, price, timestamp);
    if(m_config.m_isTimeAndSaleFeed) {
      if(auto orderEntry = Beam::Lookup(m_orderEntries, orderId)) {
        orderEntry->m_price = price;
      }
    }
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << ',' << message.m_type << ',' << orderId <<
        ',' << quantity << ',' << price << std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::HandleDeleteOrderMessage(
      const PitchMessage& message) {
    auto cursor = message.m_payload;
    auto timestamp = PitchMessage::ParseTimestamp(Beam::Store(cursor));
    auto orderId =
      std::to_string(PitchMessage::ParseUint64(Beam::Store(cursor)));
    m_marketDataFeedClient->DeleteOrder(orderId, timestamp);
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << ',' << message.m_type << ',' << orderId <<
        std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::HandleTradeMessage(
      const PitchMessage& message) {
    auto cursor = message.m_payload;
    auto timestamp = PitchMessage::ParseTimestamp(Beam::Store(cursor));
    auto symbol = PitchMessage::ParseAlphanumeric(6, Beam::Store(cursor));
    auto quantity = PitchMessage::ParseUint32(Beam::Store(cursor));
    if(quantity == 0) {
      return;
    }
    auto price = PitchMessage::ParsePrice(Beam::Store(cursor));
    auto security =
      Security(symbol, m_config.m_primaryMarket, m_config.m_country);
    auto condition = TimeAndSale::Condition();
    condition.m_code = "@";
    auto timeAndSale = TimeAndSale(
      timestamp, price, quantity, std::move(condition), m_config.m_mpid);
    m_marketDataFeedClient->Publish(
      SecurityTimeAndSale(std::move(timeAndSale), security));
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << ',' << message.m_type << ',' << symbol << ',' <<
        quantity << ',' << price << std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::Dispatch(const PitchMessage& message) {
    static const auto ADD_ORDER_MESSAGE = 0x37;
    static const auto ORDER_EXECUTED_MESSAGE = 0x38;
    static const auto REDUCE_SIZE_MESSAGE = 0x39;
    static const auto MODIFY_ORDER_MESSAGE = 0x3A;
    static const auto DELETE_ORDER_MESSAGE = 0x3C;
    static const auto TRADE_MESSAGE = 0x3D;
    if(message.m_type == ADD_ORDER_MESSAGE) {
      HandleAddOrderMessage(message);
    } else if(message.m_type == ORDER_EXECUTED_MESSAGE) {
      HandleOrderExecutedMessage(message);
    } else if(message.m_type == REDUCE_SIZE_MESSAGE) {
      HandleReduceSizeMessage(message);
    } else if(message.m_type == MODIFY_ORDER_MESSAGE) {
      HandleModifyOrderMessage(message);
    } else if(message.m_type == DELETE_ORDER_MESSAGE) {
      HandleDeleteOrderMessage(message);
    } else if(message.m_type == TRADE_MESSAGE) {
      HandleTradeMessage(message);
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::ReadLoop() {
    while(true) {
      try {
        Dispatch(m_protocolClient->Read());
      } catch(const Beam::IO::EndOfFileException&) {
        break;
      }
    }
  }
}

#endif
