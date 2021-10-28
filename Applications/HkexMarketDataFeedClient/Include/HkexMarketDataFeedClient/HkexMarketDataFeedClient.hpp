#ifndef NEXUS_HKEX_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_HKEX_MARKET_DATA_FEED_CLIENT_HPP
#include <iostream>
#include <unordered_map>
#include <Beam/IO/ConnectException.hpp>
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
      template<typename MF, typename PF>
      HkexMarketDataFeedClient(HkexConfiguration config,
        MF&& marketDataFeedClient, PF&& protocolClient);

      ~HkexMarketDataFeedClient();

      void Close();

    private:
      struct PriceLevel {
        Money m_price;
        Quantity m_quantity;
      };
      struct Entry {
        std::vector<PriceLevel> m_asks;
        std::vector<PriceLevel> m_bids;
      };
      HkexConfiguration m_config;
      Beam::GetOptionalLocalPtr<C> m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<P> m_protocolClient;
      std::unordered_map<Security, Entry> m_entries;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      HkexMarketDataFeedClient(const HkexMarketDataFeedClient&) = delete;
      HkexMarketDataFeedClient& operator =(
        const HkexMarketDataFeedClient&) = delete;
      Entry& GetEntry(const Security& security);
      Security ParseSecurity(Beam::Out<const char*> cursor) const;
      Money ParsePrice(Beam::Out<const char*> cursor) const;
      Quantity ParseQuantity32(Beam::Out<const char*> cursor) const;
      Quantity ParseQuantity64(Beam::Out<const char*> cursor) const;
      Side ParseSide(Beam::Out<const char*> cursor) const;
      boost::posix_time::ptime ToTimestamp(std::uint64_t ticks) const;
      boost::posix_time::ptime ParseTimestamp(
        Beam::Out<const char*> cursor) const;
      std::int16_t ParseShort(Beam::Out<const char*> cursor) const;
      std::uint8_t ParseByte(Beam::Out<const char*> cursor) const;
      void ParseTrade(const HkexMessage& message);
      void ParseBookUpdate(const HkexMessage& message);
      void Dispatch(const HkexMessage& message);
      void ReadLoop();
  };

  template<typename C, typename P>
  template<typename MF, typename PF>
  HkexMarketDataFeedClient<C, P>::HkexMarketDataFeedClient(
      HkexConfiguration config, MF&& marketDataFeedClient, PF&& protocolClient)
      try : m_config(std::move(config)),
            m_marketDataFeedClient(std::forward<MF>(marketDataFeedClient)),
            m_protocolClient(std::forward<PF>(protocolClient)),
            m_readLoopRoutine(Beam::Routines::Spawn([this] { ReadLoop(); })) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to initialize the HKEX market data feed client."));
  }

  template<typename C, typename P>
  HkexMarketDataFeedClient<C, P>::~HkexMarketDataFeedClient() {
    Close();
  }

  template<typename C, typename P>
  void HkexMarketDataFeedClient<C, P>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_protocolClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.Close();
  }

  template<typename C, typename P>
  typename HkexMarketDataFeedClient<C, P>::Entry&
      HkexMarketDataFeedClient<C, P>::GetEntry(const Security& security) {
    auto i = m_entries.find(security);
    if(i == m_entries.end()) {
      i = m_entries.insert(std::make_pair(security, Entry())).first;
    }
    return i->second;
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
  Quantity HkexMarketDataFeedClient<C, P>::ParseQuantity32(
      Beam::Out<const char*> cursor) const {
    auto value = Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint32_t*>(*cursor));
    *cursor += sizeof(std::uint32_t);
    return Quantity(value);
  }

  template<typename C, typename P>
  Quantity HkexMarketDataFeedClient<C, P>::ParseQuantity64(
      Beam::Out<const char*> cursor) const {
    auto value = Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint64_t*>(*cursor));
    *cursor += sizeof(std::uint64_t);
    return Quantity(value);
  }

  template<typename C, typename P>
  boost::posix_time::ptime HkexMarketDataFeedClient<C, P>::ToTimestamp(
      std::uint64_t ticks) const {
    static const auto START_POINT = boost::posix_time::ptime(
      boost::gregorian::date(1970, 1, 1), boost::posix_time::seconds(0));
    return START_POINT + boost::posix_time::microseconds(ticks / 1000);
  }

  template<typename C, typename P>
  boost::posix_time::ptime HkexMarketDataFeedClient<C, P>::ParseTimestamp(
      Beam::Out<const char*> cursor) const {
    auto value = Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint64_t*>(*cursor));
    *cursor += sizeof(std::uint64_t);
    return ToTimestamp(value);
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
  std::uint8_t HkexMarketDataFeedClient<C, P>::ParseByte(
      Beam::Out<const char*> cursor) const {
    auto value = Beam::FromLittleEndian(*reinterpret_cast<const std::uint8_t*>(
      *cursor));
    *cursor += sizeof(std::uint8_t);
    return value;
  }

  template<typename C, typename P>
  Side HkexMarketDataFeedClient<C, P>::ParseSide(
      Beam::Out<const char*> cursor) const {
    auto value = Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint16_t*>(*cursor));
    *cursor += sizeof(std::uint16_t);
    if(value == 0) {
      return Side::BID;
    }
    return Side::ASK;
  }

  template<typename C, typename P>
  void HkexMarketDataFeedClient<C, P>::ParseTrade(const HkexMessage& message) {
    auto cursor = message.m_payload;
    auto security = ParseSecurity(Beam::Store(cursor));
    cursor += sizeof(std::uint32_t);
    auto price = ParsePrice(Beam::Store(cursor));
    auto quantity = ParseQuantity32(Beam::Store(cursor));
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
    m_marketDataFeedClient->Publish(std::move(timeAndSale));
  }

  template<typename C, typename P>
  void HkexMarketDataFeedClient<C, P>::ParseBookUpdate(
      const HkexMessage& message) {
    auto cursor = message.m_payload;
    auto security = ParseSecurity(Beam::Store(cursor));
    auto& entry = GetEntry(security);
    cursor += 3;
    auto entryCount = ParseByte(Beam::Store(cursor));
    auto updateBbo = false;
    for(auto i = 0; i != entryCount; ++i) {
      auto quantity = ParseQuantity64(Beam::Store(cursor));
      auto price = ParsePrice(Beam::Store(cursor));
      cursor += sizeof(std::uint32_t);
      auto side = ParseSide(Beam::Store(cursor));
      cursor += sizeof(std::uint8_t);
      auto action = ParseByte(Beam::Store(cursor));
      cursor += 4;
      auto& levels = Pick(side, entry.m_asks, entry.m_bids);
      auto positionIterator = std::lower_bound(levels.begin(), levels.end(),
        price,
        [&] (const PriceLevel& lhs, Money rhs) {
          if(side == Side::ASK) {
            return lhs.m_price > rhs;
          } else {
            return lhs.m_price < rhs;
          }
        });
      if(action == 0 || action == 1) {
        if(positionIterator == levels.end() ||
            positionIterator->m_price != price) {
          positionIterator = levels.insert(positionIterator,
            PriceLevel{price, quantity});
        } else {
          positionIterator->m_quantity = quantity;
        }
        if(positionIterator == levels.end() - 1) {
          updateBbo = true;
          while(!levels.empty() && levels.back().m_quantity == 0) {
            levels.pop_back();
          }
        }
        m_marketDataFeedClient->Publish(
          SecurityBookQuote(BookQuote(m_config.m_mpid, true,
            m_config.m_market.m_code, Quote(price, quantity, side),
            ToTimestamp(message.m_packet->m_sendTime)), security));
      } else if(action == 2) {
        if(positionIterator != levels.end() &&
            positionIterator->m_price == price) {
          positionIterator->m_quantity = 0;
          if(positionIterator == levels.end() - 1) {
            updateBbo = true;
            while(!levels.empty() && levels.back().m_quantity == 0) {
              levels.pop_back();
            }
          }
          m_marketDataFeedClient->Publish(
            SecurityBookQuote(BookQuote(m_config.m_mpid, true,
              m_config.m_market.m_code, Quote(price, 0, side),
              ToTimestamp(message.m_packet->m_sendTime)), security));
        }
      } else if(action == 74) {
        for(auto& level : entry.m_asks) {
          m_marketDataFeedClient->Publish(
            SecurityBookQuote(BookQuote(m_config.m_mpid, true,
              m_config.m_market.m_code, Quote(level.m_price, 0, Side::ASK),
              ToTimestamp(message.m_packet->m_sendTime)), security));
        }
        for(auto& level : entry.m_bids) {
          m_marketDataFeedClient->Publish(
            SecurityBookQuote(BookQuote(m_config.m_mpid, true,
              m_config.m_market.m_code, Quote(level.m_price, 0, Side::BID),
              ToTimestamp(message.m_packet->m_sendTime)), security));
        }
        entry.m_asks.clear();
        entry.m_bids.clear();
      }
    }
    if(updateBbo) {
      auto bid = [&] {
        if(entry.m_bids.empty()) {
          return Quote(Money::ZERO, 0, Side::BID);
        }
        return Quote(entry.m_bids.back().m_price,
          entry.m_bids.back().m_quantity, Side::BID);
      }();
      auto ask = [&] {
        if(entry.m_asks.empty()) {
          return Quote(Money::ZERO, 0, Side::ASK);
        }
        return Quote(entry.m_asks.back().m_price,
          entry.m_asks.back().m_quantity, Side::ASK);
      }();
      m_marketDataFeedClient->Publish(
        SecurityBboQuote(BboQuote(std::move(bid), std::move(ask),
          ToTimestamp(message.m_packet->m_sendTime)), security));
    }
  }

  template<typename C, typename P>
  void HkexMarketDataFeedClient<C, P>::Dispatch(const HkexMessage& message) {
    switch(message.m_type) {
      case HkexMessage::TRADE:
        ParseTrade(message);
        break;
      case HkexMessage::BOOK_UPDATE:
        ParseBookUpdate(message);
        break;
    }
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
