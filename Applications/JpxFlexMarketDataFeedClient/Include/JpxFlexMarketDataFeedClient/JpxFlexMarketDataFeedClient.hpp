#ifndef NEXUS_JPX_FLEX_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_JPX_FLEX_MARKET_DATA_FEED_CLIENT_HPP
#include <utility>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "JpxFlexMarketDataFeedClient/JpxFlexConfiguration.hpp"
#include "JpxFlexMarketDataFeedClient/JpxFlexMessage.hpp"

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
      struct PriceLevel {
        Money m_price;
        Quantity m_quantity;
      };
      struct BboEntry {
        std::vector<PriceLevel> m_asks;
        std::vector<PriceLevel> m_bids;
      };
      JpxFlexConfiguration m_config;
      Beam::GetOptionalLocalPtr<C> m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<P> m_protocolClient;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      std::unordered_map<Security, BboEntry> m_bboEntries;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      boost::posix_time::ptime ParseTimestamp(Beam::Out<const char*> source);
      Security ParseSecurity(const JpxFlexPacket& packet);
      Money ParsePrice(Beam::Out<const char*> source);
      Quantity ParseQuantity(Beam::Out<const char*> source);
      void HandleCurrentPriceMessage(const JpxFlexMessage& message);
      void HandleTradingVolumeMessage(const JpxFlexMessage& message);
      void HandleTurnoverMessage(const JpxFlexMessage& message);
      void HandleQuoteMessage(const JpxFlexMessage& message, Side side);
      void Dispatch(const JpxFlexMessage& message);
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
  boost::posix_time::ptime JpxFlexMarketDataFeedClient<C, P>::ParseTimestamp(
      Beam::Out<const char*> source) {
    auto remainingSize = std::size_t{12};
    auto hours = ParseNumber<std::uint8_t>(Beam::Store(source),
      Beam::Store(remainingSize), 2);
    auto minutes = ParseNumber<std::uint8_t>(Beam::Store(source),
      Beam::Store(remainingSize), 2);
    auto seconds = ParseNumber<std::uint8_t>(Beam::Store(source),
      Beam::Store(remainingSize), 2);
    auto microseconds = ParseNumber<std::uint32_t>(Beam::Store(source),
      Beam::Store(remainingSize), 6);
    auto timeOfDay = boost::posix_time::hours(hours) +
      boost::posix_time::minutes(minutes) +
      boost::posix_time::seconds(seconds) +
      boost::posix_time::microseconds(microseconds);
    auto timestamp = boost::posix_time::ptime(
      boost::posix_time::second_clock::universal_time().date(),
      timeOfDay + m_config.m_utcOffset);
    return timestamp;
  }

  template<typename C, typename P>
  Security JpxFlexMarketDataFeedClient<C, P>::ParseSecurity(
      const JpxFlexPacket& packet) {
    return Security(std::string(packet.m_issueCode.GetData() + 7, 4),
      packet.m_exchange, DefaultCountries::JP());
  }

  template<typename C, typename P>
  Money JpxFlexMarketDataFeedClient<C, P>::ParsePrice(
      Beam::Out<const char*> source) {
    auto remainingSize = std::size_t{16};
    auto flag = ParseNumber<std::uint8_t>(Beam::Store(source),
      Beam::Store(remainingSize), 1);
    auto price = ParseNumber<std::uint32_t>(Beam::Store(source),
      Beam::Store(remainingSize), 10) * Money::ONE;
    price += (ParseNumber<std::uint16_t>(Beam::Store(source),
      Beam::Store(remainingSize), 4) * Money::ONE) / 10000;
    ++*source;
    return price;
  }

  template<typename C, typename P>
  Quantity JpxFlexMarketDataFeedClient<C, P>::ParseQuantity(
      Beam::Out<const char*> source) {
    auto remainingSize = std::size_t{16};
    auto flag = ParseNumber<std::uint8_t>(Beam::Store(source),
      Beam::Store(remainingSize), 1);
    auto value = ParseNumber<std::uint64_t>(Beam::Store(source),
      Beam::Store(remainingSize), 14);
    while(flag != 0) {
      value *= 10;
      --flag;
    }
    return value;
  }

  template<typename C, typename P>
  void JpxFlexMarketDataFeedClient<C, P>::HandleCurrentPriceMessage(
      const JpxFlexMessage& message) {
    auto security = ParseSecurity(*message.m_packet);
    std::cout << "Current: " << security << " " <<
      std::string(message.m_payload, message.m_size) << std::endl;
  }

  template<typename C, typename P>
  void JpxFlexMarketDataFeedClient<C, P>::HandleTradingVolumeMessage(
      const JpxFlexMessage& message) {
    auto security = ParseSecurity(*message.m_packet);
    auto source = message.m_payload;
    source += 3;
    auto volume = ParseQuantity(Beam::Store(source));
    auto timestamp = ParseTimestamp(Beam::Store(source));
    std::cout << "Volume: " << security << " " << volume << " " << timestamp <<
      std::endl;
  }

  template<typename C, typename P>
  void JpxFlexMarketDataFeedClient<C, P>::HandleTurnoverMessage(
      const JpxFlexMessage& message) {
    auto security = ParseSecurity(*message.m_packet);
    auto source = message.m_payload;
    source += 3;
    auto turnover = ParseQuantity(Beam::Store(source)) * Money::ONE;
    auto timestamp = ParseTimestamp(Beam::Store(source));
    std::cout << "Turnover: " << security << " " << turnover << " " <<
      timestamp << std::endl;
  }

  template<typename C, typename P>
  void JpxFlexMarketDataFeedClient<C, P>::HandleQuoteMessage(
      const JpxFlexMessage& message, Side side) {
    auto security = ParseSecurity(*message.m_packet);
    auto source = message.m_payload;
    source += 3;
    auto price = ParsePrice(Beam::Store(source));
    auto timestamp = ParseTimestamp(Beam::Store(source));
    auto flag = *source;
    if(flag != '1' && flag != '2' && flag != ' ') {
      return;
    }
    source += 2;
    auto quantity = ParseQuantity(Beam::Store(source));
    auto& bboEntry = Beam::GetOrInsert(m_bboEntries, security,
      [] {
        return BboEntry();
      });
    auto& levels = Pick(side, bboEntry.m_asks, bboEntry.m_bids);
    auto positionIterator = std::lower_bound(levels.begin(),
      levels.end(), price,
      [&] (const PriceLevel& lhs, Money rhs) {
        if(side == Side::ASK) {
          return lhs.m_price < rhs;
        } else {
          return lhs.m_price > rhs;
        }
      });
    if(positionIterator == levels.end() ||
        positionIterator->m_price != price) {
      if(quantity == 0) {
        return;
      }
      auto level = PriceLevel();
      level.m_price = price;
      level.m_quantity = quantity;
      positionIterator = levels.insert(positionIterator, level);
    } else {
      auto& level = *positionIterator;
      level.m_quantity = quantity;
      if(level.m_quantity == 0) {
        positionIterator = levels.erase(positionIterator);
      }
    }
    m_marketDataFeedClient->SetBookQuote(SecurityBookQuote(
      BookQuote(m_config.m_mpid, true, m_config.m_disseminatingMarket,
      Quote(price, quantity, side), timestamp), security));
    if(positionIterator == levels.begin()) {
      auto ask = Quote();
      ask.m_side = Side::ASK;
      if(bboEntry.m_asks.empty()) {
        ask.m_price = Money::ZERO;
        ask.m_size = 0;
      } else {
        ask.m_price = bboEntry.m_asks.front().m_price;
        ask.m_size = bboEntry.m_asks.front().m_quantity;
      }
      auto bid = Quote();
      bid.m_side = Side::BID;
      if(bboEntry.m_bids.empty()) {
        bid.m_price = Money::ZERO;
        bid.m_size = 0;
      } else {
        bid.m_price = bboEntry.m_bids.front().m_price;
        bid.m_size = bboEntry.m_bids.front().m_quantity;
      }
      auto bbo = BboQuote(bid, ask, timestamp);
      m_marketDataFeedClient->PublishBboQuote(SecurityBboQuote(bbo, security));
    }
  }

  template<typename C, typename P>
  void JpxFlexMarketDataFeedClient<C, P>::Dispatch(
      const JpxFlexMessage& message) {
    if(message.m_packet->m_issueLargeClassification !=
        JpxFlexPacket::IssueLargeClassification::STOCK_RELATED) {
      return;
    }
    if(message.m_type == JpxFlexMessage::Type::CURRENT_PRICE) {
      HandleCurrentPriceMessage(message);
    } else if(message.m_type == JpxFlexMessage::Type::TRADING_VOLUME) {
      HandleTradingVolumeMessage(message);
    } else if(message.m_type == JpxFlexMessage::Type::TURNOVER) {
      HandleTurnoverMessage(message);
    } else if(message.m_type == JpxFlexMessage::Type::ASK_QUOTE) {
      HandleQuoteMessage(message, Side::ASK);
    } else if(message.m_type == JpxFlexMessage::Type::BID_QUOTE) {
      HandleQuoteMessage(message, Side::BID);
    }
  }

  template<typename C, typename P>
  void JpxFlexMarketDataFeedClient<C, P>::ReadLoop() {
    auto lastSequence = std::uint32_t{0};
    auto sequence = std::uint32_t{0};
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
