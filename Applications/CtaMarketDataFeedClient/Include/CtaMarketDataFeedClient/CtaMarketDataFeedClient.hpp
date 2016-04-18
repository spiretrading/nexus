#ifndef NEXUS_CTAMARKETDATAFEEDCLIENT_HPP
#define NEXUS_CTAMARKETDATAFEEDCLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "CtaMarketDataFeedClient/CtaConfiguration.hpp"
#include "CtaMarketDataFeedClient/CtaMessage.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class CtaMarketDataFeedClient
      \brief Parses packets from a CTA market data feed.
      \tparam MarketDataFeedClientType The type of MarketDataFeedClient used to
              update the MarketDataServer.
      \tparam ProtocolClientType The type of client receiving messages.
   */
  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  class CtaMarketDataFeedClient : private boost::noncopyable {
    public:

      //! The type of MarketDataFeedClient used to update the MarketDataServer.
      using MarketDataFeedClient =
        Beam::GetTryDereferenceType<MarketDataFeedClientType>;

      //! The type of client receiving messages.
      using ProtocolClient = Beam::GetTryDereferenceType<ProtocolClientType>;

      //! Constructs a CtaMarketDataFeedClient.
      /*!
        \param config The configuration to use.
        \param marketDataFeedClient Initializes the MarketDataFeedClient.
        \param protocolClient The client receiving messages.
      */
      template<typename MarketDataFeedClientForward,
        typename ProtocolClientForward>
      CtaMarketDataFeedClient(const CtaConfiguration& config,
        MarketDataFeedClientForward&& marketDataFeedClient,
        ProtocolClientForward&& protocolClient);

      ~CtaMarketDataFeedClient();

      void Open();

      void Close();

    private:
      CtaConfiguration m_config;
      Beam::GetOptionalLocalPtr<MarketDataFeedClientType>
        m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<ProtocolClientType> m_protocolClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      void Dispatch(const CtaMessage& message);
      void ReadLoop();
  };

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  template<typename MarketDataFeedClientForward, typename ProtocolClientForward>
  CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      CtaMarketDataFeedClient(const CtaConfiguration& config,
      MarketDataFeedClientForward&& marketDataFeedClient,
      ProtocolClientForward&& protocolClient)
      : m_config(config),
        m_marketDataFeedClient{std::forward<MarketDataFeedClientForward>(
          marketDataFeedClient)},
        m_protocolClient{std::forward<ProtocolClientForward>(protocolClient)} {}

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ~CtaMarketDataFeedClient() {
    Close();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataFeedClient->Open();
      m_protocolClient->Open();
      m_readLoopRoutine = Beam::Routines::Spawn(
        std::bind(&CtaMarketDataFeedClient::ReadLoop, this));
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Shutdown() {
    m_protocolClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.SetClosed();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Dispatch(const CtaMessage& message) {}

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void CtaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ReadLoop() {
    while(true) {
      try {
        auto message = m_protocolClient->Read();
        if(m_config.m_isLoggingMessages) {
          std::cout << std::string{message.m_data, message.m_dataLength} <<
            std::endl;
        }
        Dispatch(message);
      } catch(const Beam::IO::EndOfFileException&) {
        break;
      }
    }
  }
}
}

#endif
