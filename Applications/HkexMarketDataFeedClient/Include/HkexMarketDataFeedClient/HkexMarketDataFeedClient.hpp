#ifndef NEXUS_HKEX_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_HKEX_MARKET_DATA_FEED_CLIENT_HPP
#include <iostream>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include "HkexMarketDataFeedClient/HkexConfiguration.hpp"
#include "HkexMarketDataFeedClient/HkexMessage.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /**
   * Parses packets from the HKEX market data feed.
   * @param <C> The type of MarketDataFeedClient used to update the
   *            MarketDataServer.
   * @param <P> The type of client receiving HKEX messages.
   */
  template<typename C, typename P>
  class HkexMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the
       * MarketDataServer.
       */
      using MarketDataFeedClient = Beam::GetTryDereferenceType<C>;

      /** The type of client receiving HKEX messages. */
      using ProtocolClient = Beam::GetTryDereferenceType<P>;

      /**
       * Constructs an HkexMarketDataFeedClient.
       * @param config The configuration to use.
       * @param marketDataFeedClient Initializes the MarketDataFeedClient.
       * @param protocolClient The client receiving HKEX messages.
       */
      template<typename MarketDataFeedClientForward,
        typename ProtocolClientForward>
      HkexMarketDataFeedClient(HkexConfiguration config,
        MarketDataFeedClientForward&& marketDataFeedClient,
        ProtocolClientForward&& protocolClient);

      ~HkexMarketDataFeedClient();

      void Open();

      void Close();

    private:
      HkexConfiguration m_config;
      Beam::GetOptionalLocalPtr<C> m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<P> m_protocolClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      void Dispatch(const HkexMessage& message);
      void ReadLoop();
  };

  template<typename C, typename P>
  template<typename MarketDataFeedClientForward, typename ProtocolClientForward>
  HkexMarketDataFeedClient<C, P>::HkexMarketDataFeedClient(
    HkexConfiguration config,
    MarketDataFeedClientForward&& marketDataFeedClient,
    ProtocolClientForward&& protocolClient)
    : m_config(std::move(config)),
      m_marketDataFeedClient(std::forward<MarketDataFeedClientForward>(
        marketDataFeedClient)),
      m_protocolClient(std::forward<ProtocolClientForward>(protocolClient)) {}

  template<typename C, typename P>
  HkexMarketDataFeedClient<C, P>::~HkexMarketDataFeedClient() {
    Close();
  }

  template<typename C, typename P>
  void HkexMarketDataFeedClient<C, P>::Open() {
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
  void HkexMarketDataFeedClient<C, P>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C, typename P>
  void HkexMarketDataFeedClient<C, P>::Shutdown() {
    m_protocolClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.SetClosed();
  }

  template<typename C, typename P>
  void HkexMarketDataFeedClient<C, P>::Dispatch(const HkexMessage& message) {
    std::cout << message.m_type << " " << message.m_size << std::endl;
  }

  template<typename C, typename P>
  void HkexMarketDataFeedClient<C, P>::ReadLoop() {
    auto lastSequence = std::uint32_t{0};
    auto sequence = std::uint32_t{};
    while(true) {
      try {
        auto message = m_protocolClient->Read(Beam::Store(sequence));
        if(lastSequence != 0 && sequence != lastSequence + 1) {
          std::cout << "Packets dropped: " << (lastSequence + 1) << " - " <<
            (sequence - 1) << std::endl;
        }
        Dispatch(message);
      } catch(const Beam::IO::EndOfFileException&) {
        break;
      }
    }
  }
}

#endif
