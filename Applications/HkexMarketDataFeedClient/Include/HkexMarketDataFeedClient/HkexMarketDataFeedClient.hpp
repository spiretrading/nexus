#ifndef NEXUS_HKEX_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_HKEX_MARKET_DATA_FEED_CLIENT_HPP
#include <iostream>
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include "HkexMarketDataFeedClient/HkexConfiguration.hpp"
#include "HkexMarketDataFeedClient/HkexMessage.hpp"
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
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
      struct Entry {
        BboQuote m_bbo;
      };
      HkexConfiguration m_config;
      Beam::GetOptionalLocalPtr<C> m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<P> m_protocolClient;
      std::unordered_map<Security, Entry> m_entries;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      Security ParseSecurity(Beam::Out<const char*> cursor) const;
      Money ParsePrice(Beam::Out<const char*> cursor) const;
      Quantity ParseQuantity(Beam::Out<const char*> cursor) const;
      boost::posix_time::ptime ParseTimestamp(
        Beam::Out<const char*> cursor) const;
      std::int16_t ParseShort(Beam::Out<const char*> cursor) const;
      void ParseTrade(const HkexMessage& message);
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
  Security HkexMarketDataFeedClient<C, P>::ParseSecurity(
      Beam::Out<const char*> cursor) const {
    auto security = Security(std::to_string(Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint32_t*>(*cursor))),
      m_config.m_market.m_code, m_config.m_market.m_countryCode);
    *cursor += sizeof(std::uint32_t);
    return security;
  }

  template<typename C, typename P>
  Money HkexMarketDataFeedClient<C, P>::ParsePrice(
      Beam::Out<const char*> cursor) const {
    auto value = Beam::FromLittleEndian(
      *reinterpret_cast<const std::int32_t*>(*cursor));
    *cursor += sizeof(std::int32_t);
    return value * (Money::ONE / 1000);
  }

  template<typename C, typename P>
  Quantity HkexMarketDataFeedClient<C, P>::ParseQuantity(
      Beam::Out<const char*> cursor) const {
    auto value = Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint32_t*>(*cursor));
    *cursor += sizeof(std::uint32_t);
    return Quantity(value);
  }

  template<typename C, typename P>
  boost::posix_time::ptime HkexMarketDataFeedClient<C, P>::ParseTimestamp(
      Beam::Out<const char*> cursor) const {
    static const auto START_POINT = boost::posix_time::ptime(
      boost::gregorian::date(1970, 1, 1), boost::posix_time::seconds(0));
    auto value = Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint64_t*>(*cursor));
    return START_POINT + boost::posix_time::microseconds(value / 1000);
  }

  template<typename C, typename P>
  std::int16_t HkexMarketDataFeedClient<C, P>::ParseShort(
      Beam::Out<const char*> cursor) const {
    auto value = Beam::FromLittleEndian(
      *reinterpret_cast<const std::int16_t*>(*cursor));
    *cursor += sizeof(std::int16_t);
    return value;
  }

  template<typename C, typename P>
  void HkexMarketDataFeedClient<C, P>::ParseTrade(const HkexMessage& message) {
    auto cursor = message.m_payload;
    auto security = ParseSecurity(Beam::Store(cursor));
    cursor += sizeof(std::uint32_t);
    auto price = ParsePrice(Beam::Store(cursor));
    auto quantity = ParseQuantity(Beam::Store(cursor));
    auto type = ParseShort(Beam::Store(cursor));
    auto code = [&] {
      switch(type) {
        case 0:
          return " ";
        case 4:
          return "P";
        case 22:
          return "M";
        case 100:
          return "Y";
        case 101:
          return "X";
        case 102:
          return "D";
        case 103:
          return "U";
        case 104:
          return " ";
        default:
          return " ";
      }
    }();
    cursor += 2;
    auto timestamp = ParseTimestamp(Beam::Store(cursor));
    auto timeAndSale = SecurityTimeAndSale(
      TimeAndSale(timestamp, price, quantity,
      TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, code),
      m_config.m_market.m_displayName), std::move(security));
    m_marketDataFeedClient->PublishTimeAndSale(std::move(timeAndSale));
  }

  template<typename C, typename P>
  void HkexMarketDataFeedClient<C, P>::Dispatch(const HkexMessage& message) {
    switch(message.m_type) {
      case HkexMessage::TRADE:
        ParseTrade(message);
        break;
    }
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
