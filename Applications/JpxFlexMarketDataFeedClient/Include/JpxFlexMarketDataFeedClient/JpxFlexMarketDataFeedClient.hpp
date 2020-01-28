#ifndef NEXUS_JPX_FLEX_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_JPX_FLEX_MARKET_DATA_FEED_CLIENT_HPP
#include <utility>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include "JpxFlexMarketDataFeedClient/JpxFlexConfiguration.hpp"

namespace Nexus::MarketDataService {

  /**
   * Parses packets from the JPX Flex market data feed.
   * @param <C> The type of MarketDataFeedClient used to update the
   *            MarketDataServer.
   * @param <P> The type of client receiving JPX Flex messages.
   */
  template<typename C, typename P>
  class JpxFlexMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the
       * MarketDataServer.
       */
      using MarketDataFeedClient = Beam::GetTryDereferenceType<C>;

      /** The type of client receiving JPX Flex messages. */
      using ProtocolClient = Beam::GetTryDereferenceType<P>;

      /**
       * Constructs an JpxFlexMarketDataFeedClient.
       * @param config The configuration to use.
       * @param marketDataFeedClient Initializes the MarketDataFeedClient.
       * @param protocolClient The client receiving HKEX messages.
       */
      template<typename MarketDataFeedClientForward,
        typename ProtocolClientForward>
      JpxFlexMarketDataFeedClient(JpxFlexConfiguration config,
        MarketDataFeedClientForward&& marketDataFeedClient,
        ProtocolClientForward&& protocolClient);

      ~JpxFlexMarketDataFeedClient();

      void Open();

      void Close();

    private:
      JpxFlexConfiguration m_config;
      Beam::GetOptionalLocalPtr<C> m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<P> m_protocolClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      void ReadLoop();
  };

  template<typename C, typename P>
  template<typename MarketDataFeedClientForward, typename ProtocolClientForward>
  JpxFlexMarketDataFeedClient<C, P>::JpxFlexMarketDataFeedClient(
    JpxFlexConfiguration config,
    MarketDataFeedClientForward&& marketDataFeedClient,
    ProtocolClientForward&& protocolClient)
    : m_config(std::move(config)),
      m_marketDataFeedClient(std::forward<MarketDataFeedClientForward>(
        marketDataFeedClient)),
      m_protocolClient(std::forward<ProtocolClientForward>(protocolClient)) {}

  template<typename C, typename P>
  JpxFlexMarketDataFeedClient<C, P>::~JpxFlexMarketDataFeedClient() {
    Close();
  }

  template<typename C, typename P>
  void JpxFlexMarketDataFeedClient<C, P>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataFeedClient->Open();
      m_protocolClient->Open();
      m_readLoopRoutine = Beam::Routines::Spawn([=] { ReadLoop(); });
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C, typename P>
  void JpxFlexMarketDataFeedClient<C, P>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C, typename P>
  void JpxFlexMarketDataFeedClient<C, P>::Shutdown() {
    m_protocolClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.SetClosed();
  }

  template<typename C, typename P>
  void JpxFlexMarketDataFeedClient<C, P>::ReadLoop() {}
}

#endif
