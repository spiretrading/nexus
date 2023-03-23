#ifndef NEXUS_CHIA_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_CHIA_MARKET_DATA_FEED_CLIENT_HPP
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
      ChiaConfiguration m_config;
      Beam::GetOptionalLocalPtr<M> m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<P> m_protocolClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      ChiaMarketDataFeedClient(const ChiaMarketDataFeedClient&) = delete;
      ChiaMarketDataFeedClient& operator =(
        const ChiaMarketDataFeedClient&) = delete;
      void HandleAddOrderMessage(const PitchMessage& message);
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
    auto orderId = PitchMessage::ParseUint64(Beam::Store(cursor));
    auto side = PitchMessage::ParseSide(Beam::Store(cursor));
    auto quantity = PitchMessage::ParseUint32(Beam::Store(cursor));
    if(quantity == 0) {
      return;
    }
    auto symbol = PitchMessage::ParseAlphanumeric(6, Beam::Store(cursor));
    auto price = PitchMessage::ParsePrice(Beam::Store(cursor));
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << ',' << message.m_type << ',' << orderId <<
        ',' << side << ',' << quantity << ',' << symbol << ',' << price <<
        std::endl;
    }
  }

  template<typename M, typename P>
  void ChiaMarketDataFeedClient<M, P>::Dispatch(const PitchMessage& message) {
    static const auto ADD_ORDER_MESSAGE = 0x37;
    if(message.m_type == ADD_ORDER_MESSAGE) {
      HandleAddOrderMessage(message);
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
