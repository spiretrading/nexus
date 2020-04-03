#ifndef NEXUS_ASX_ITCH_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_ASX_ITCH_MARKET_DATA_FEED_CLIENT_HPP
#include <cstdint>
#include <string>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/Endian.hpp>
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "AsxItchMarketDataFeedClient/AsxItchConfiguration.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MoldUdp64/MoldUdp64Client.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpClient.hpp"

namespace Nexus::MarketDataService {

  /** The type of product. */
  enum class ProductType {

    /** An option. */
    OPTION,

    /** A future. */
    FUTURE,

    /** A cash equity. */
    EQUITY,
  };

  /** Stores info needed to maintain a Security's order book. */
  struct OrderBookDirectory {

    /** The order book id. */
    std::uint32_t m_id;

    /** The type of product. */
    ProductType m_productType;

    /** Details about the Security represented by this order book. */
    SecurityInfo m_security;

    /** The book's currency. */
    CurrencyId m_currency;

    /** The number of decimal places used in the book's price. */
    std::uint16_t m_priceDecimalPlaces;

    /** The number of decimal places used in the book's nominal value. */
    std::uint16_t m_valueDecimalPlaces;

    /** The size of an odd-lot. */
    std::uint32_t m_oddLotSize;

    /** The size of a block-lot. */
    std::uint64_t m_blockLotSize;
  };

  /**
   * Parses packets from the ASX ITCH feed.
   * @param <M> The type of MarketDataFeedClient used to update the
   *            MarketDataServer.
   * @param <I> The type of client receiving ITCH messages.
   * @param <G> The type of client connecting to Glimpse.
   */
  template<typename M, typename I, typename G>
  class AsxItchMarketDataFeedClient : private boost::noncopyable {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::GetTryDereferenceType<M>;

      /** The type of client receiving ITCH messages. */
      using ItchClient = Beam::GetTryDereferenceType<I>;

      /** The type of client connecting to Glimpse. */
      using GlimpseClient = Beam::GetTryDereferenceType<G>;

      /**
       * Constructs a AsxItchMarketDataFeedClient.
       * @param config The configuration to use.
       * @param marketDataFeedClient Initializes the MarketDataFeedClient.
       * @param itchClient The client receiving ITCH messages.
       * @param glimpseClient The client connecting to Glimpse.
       */
      template<typename MF, typename IF, typename GF>
      AsxItchMarketDataFeedClient(const AsxItchConfiguration& config,
        Beam::Ref<CurrencyDatabase> currencyDatabase, MF&& marketDataFeedClient,
        IF&& itchClient, GF&& glimpseClient);

      ~AsxItchMarketDataFeedClient();

      void Open();

      void Close();

    private:
      struct OrderEntry {
        std::string m_mpid;
        Money m_price;
        Quantity m_remainingQuantity;
      };
      struct PriceLevel {
        Money m_price;
        Quantity m_quantity;
      };
      struct BboEntry {
        std::vector<PriceLevel> m_asks;
        std::vector<PriceLevel> m_bids;
      };
      AsxItchConfiguration m_config;
      CurrencyDatabase* m_currencyDatabase;
      Beam::GetOptionalLocalPtr<M> m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<I> m_itchClient;
      Beam::GetOptionalLocalPtr<G> m_glimpseClient;
      boost::posix_time::ptime m_lastTimePoint;
      std::unordered_map<std::uint32_t, OrderBookDirectory>
        m_orderBookDirectories;
      std::unordered_map<Security, BboEntry> m_bboEntries;
      std::unordered_map<std::string, OrderEntry> m_orderEntries;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      boost::posix_time::ptime ParseTimestamp(Beam::Out<const char*> cursor);
      std::uint8_t ParseChar(Beam::Out<const char*> cursor);
      std::uint8_t ParseInt8(Beam::Out<const char*> cursor);
      std::uint16_t ParseInt16(Beam::Out<const char*> cursor);
      std::uint32_t ParseInt32(Beam::Out<const char*> cursor);
      std::uint64_t ParseInt64(Beam::Out<const char*> cursor);
      std::string ParseAlpha(std::size_t size, Beam::Out<const char*> cursor);
      Money ParsePrice(const OrderBookDirectory& directory,
        Beam::Out<const char*> cursor);
      Side ParseSide(Beam::Out<const char*> cursor);
      std::string BuildOrderKey(const Security& security, Side side,
        std::uint64_t orderId);
      void UpdateBbo(const Security& security, Side side, Money price,
        Quantity delta, const boost::posix_time::ptime& timestamp);
      void HandleSecondsMessage(const MoldUdp64::MoldUdp64Message& message);
      void HandleAddOrderMessage(bool isAnonymous,
        const MoldUdp64::MoldUdp64Message& message);
      void HandleOrderExecutedMessage(
        const MoldUdp64::MoldUdp64Message& message);
      void HandleOrderExecutedAtPriceMessage(
        const MoldUdp64::MoldUdp64Message& message);
      void HandleOrderReplaceMessage(
        const MoldUdp64::MoldUdp64Message& message);
      void HandleOrderDeleteMessage(
        const MoldUdp64::MoldUdp64Message& message);
      void HandleTradeMessage(const MoldUdp64::MoldUdp64Message& message);
      void HandleOrderBookDirectoryMessage(
        const MoldUdp64::MoldUdp64Message& message);
      void Dispatch(const MoldUdp64::MoldUdp64Message& message);
      void ReadLoop();
  };

  template<typename M, typename I, typename G>
  template<typename MF, typename IF, typename GF>
  AsxItchMarketDataFeedClient<M, I, G>::AsxItchMarketDataFeedClient(
    const AsxItchConfiguration& config,
    Beam::Ref<CurrencyDatabase> currencyDatabase, MF&& marketDataFeedClient,
    IF&& itchClient, GF&& glimpseClient)
    : m_config(config),
      m_currencyDatabase(currencyDatabase.Get()),
      m_marketDataFeedClient(std::forward<MF>(marketDataFeedClient)),
      m_itchClient(std::forward<IF>(itchClient)),
      m_glimpseClient(std::forward<GF>(glimpseClient)),
      m_lastTimePoint(boost::posix_time::not_a_date_time) {}

  template<typename M, typename I, typename G>
  AsxItchMarketDataFeedClient<M, I, G>::~AsxItchMarketDataFeedClient() {
    Close();
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataFeedClient->Open();
      m_itchClient->Open();
      m_glimpseClient->Login(m_config.m_glimpseUsername,
        m_config.m_glimpsePassword);
      m_readLoopRoutine = Beam::Routines::Spawn(
        std::bind(&AsxItchMarketDataFeedClient::ReadLoop, this));
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::Shutdown() {
    m_glimpseClient->Close();
    m_itchClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.SetClosed();
  }

  template<typename M, typename I, typename G>
  boost::posix_time::ptime AsxItchMarketDataFeedClient<M, I, G>::ParseTimestamp(
      Beam::Out<const char*> cursor) {
    auto nanoseconds = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint32_t*>(*cursor));
    *cursor += sizeof(std::uint32_t);
    if(m_lastTimePoint == boost::posix_time::not_a_date_time) {
      return m_lastTimePoint;
    }
    return m_lastTimePoint +
      boost::posix_time::microseconds(nanoseconds / 1000);
  }

  template<typename M, typename I, typename G>
  std::uint8_t AsxItchMarketDataFeedClient<M, I, G>::ParseChar(
      Beam::Out<const char*> cursor) {
    auto result = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint8_t*>(*cursor));
    *cursor += sizeof(std::uint8_t);
    return result;
  }

  template<typename M, typename I, typename G>
  std::uint8_t AsxItchMarketDataFeedClient<M, I, G>::ParseInt8(
      Beam::Out<const char*> cursor) {
    auto result = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint8_t*>(*cursor));
    *cursor += sizeof(std::uint8_t);
    return result;
  }

  template<typename M, typename I, typename G>
  std::uint16_t AsxItchMarketDataFeedClient<M, I, G>::ParseInt16(
      Beam::Out<const char*> cursor) {
    auto result = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint16_t*>(*cursor));
    *cursor += sizeof(std::uint16_t);
    return result;
  }

  template<typename M, typename I, typename G>
  std::uint32_t AsxItchMarketDataFeedClient<M, I, G>::ParseInt32(
      Beam::Out<const char*> cursor) {
    auto result = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint32_t*>(*cursor));
    *cursor += sizeof(std::uint32_t);
    return result;
  }

  template<typename M, typename I, typename G>
  std::uint64_t AsxItchMarketDataFeedClient<M, I, G>::ParseInt64(
      Beam::Out<const char*> cursor) {
    auto result = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint64_t*>(*cursor));
    *cursor += sizeof(std::uint64_t);
    return result;
  }

  template<typename M, typename I, typename G>
  std::string AsxItchMarketDataFeedClient<M, I, G>::ParseAlpha(std::size_t size,
      Beam::Out<const char*> cursor) {
    if(size == 0) {
      return std::string{};
    }
    auto lastCharacter = *cursor + size - 1;
    while(true) {
      if(*lastCharacter == ' ') {
        if(lastCharacter == *cursor) {
          *cursor += size;
          return std::string{};
        } else {
          --lastCharacter;
        }
      } else {
        break;
      }
    }
    auto result = std::string(*cursor, lastCharacter + 1);
    *cursor += size;
    return result;
  }

  template<typename M, typename I, typename G>
  Side AsxItchMarketDataFeedClient<M, I, G>::ParseSide(
      Beam::Out<const char*> cursor) {
    auto s = ParseChar(Beam::Store(cursor));
    if(s == 'S') {
      return Side::ASK;
    } else if(s == 'B') {
      return Side::BID;
    }
    return Side::NONE;
  }

  template<typename M, typename I, typename G>
  Money AsxItchMarketDataFeedClient<M, I, G>::ParsePrice(
      const OrderBookDirectory& directory, Beam::Out<const char*> cursor) {
    auto PowerOfTen =
      [] (std::uint16_t exponent) {
        Quantity result = 1;
        for(auto i = std::uint16_t{0}; i < exponent; ++i) {
          result *= 10;
        }
        return result;
      };
    auto price = Money{ParseInt32(Beam::Store(cursor)) /
      (100 * PowerOfTen(directory.m_priceDecimalPlaces))};
    return price;
  }

  template<typename M, typename I, typename G>
  std::string AsxItchMarketDataFeedClient<M, I, G>::BuildOrderKey(
      const Security& security, Side side, std::uint64_t orderId) {
    auto result = security.GetSymbol();
    result += '-';
    if(side == Side::ASK) {
      result += 'A';
    } else {
      result += 'B';
    }
    result += '-';
    result += std::to_string(orderId);
    return result;
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::HandleSecondsMessage(
      const MoldUdp64::MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto epochTime = static_cast<std::time_t>(Beam::FromBigEndian(
      *reinterpret_cast<const std::uint32_t*>(cursor)));
    m_lastTimePoint = boost::posix_time::from_time_t(epochTime);
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::UpdateBbo(const Security& security,
      Side side, Money price, Quantity delta,
      const boost::posix_time::ptime& timestamp) {
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
      if(delta <= 0) {
        return;
      }
      auto level = PriceLevel();
      level.m_price = price;
      level.m_quantity = delta;
      positionIterator = levels.insert(positionIterator, level);
    } else {
      auto& level = *positionIterator;
      level.m_quantity += delta;
      if(level.m_quantity <= 0) {
        positionIterator = levels.erase(positionIterator);
      }
    }
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

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::HandleAddOrderMessage(
      bool isAnonymous, const MoldUdp64::MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(Beam::Store(cursor));
    auto orderId = ParseInt64(Beam::Store(cursor));
    auto orderBookId = ParseInt32(Beam::Store(cursor));
    auto directory = Beam::Retrieve(m_orderBookDirectories, orderBookId);
    if(!directory.is_initialized()) {
      return;
    }
    auto side = ParseSide(Beam::Store(cursor));
    auto position = ParseInt32(Beam::Store(cursor));
    auto quantity = ParseInt64(Beam::Store(cursor));
    if(quantity == 0) {
      return;
    }
    auto price = ParsePrice(*directory, Beam::Store(cursor));
    auto type = ParseInt16(Beam::Store(cursor));
    auto lot = ParseInt8(Beam::Store(cursor));
    auto mpid = [&] {
      if(isAnonymous) {
        return std::string("AU000");
      } else {
        return ParseAlpha(7, Beam::Store(cursor));
      }
    }();
    auto orderKey = BuildOrderKey(directory->m_security.m_security, side,
      orderId);
    auto orderEntry = OrderEntry();
    orderEntry.m_mpid = mpid;
    orderEntry.m_price = price;
    orderEntry.m_remainingQuantity = quantity;
    m_orderEntries[orderKey] = orderEntry;
    m_marketDataFeedClient->AddOrder(directory->m_security.m_security,
      m_config.m_market.m_code, mpid, false, orderKey, side, price, quantity,
      timestamp);
    UpdateBbo(directory->m_security.m_security, side, price, quantity,
      timestamp);
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::HandleOrderExecutedMessage(
      const MoldUdp64::MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(Beam::Store(cursor));
    auto orderId = ParseInt64(Beam::Store(cursor));
    auto orderBookId = ParseInt32(Beam::Store(cursor));
    auto directory = Beam::Retrieve(m_orderBookDirectories, orderBookId);
    if(!directory.is_initialized()) {
      return;
    }
    auto side = ParseSide(Beam::Store(cursor));
    auto executedQuantity = static_cast<std::int64_t>(
      ParseInt64(Beam::Store(cursor)));
    auto orderKey = BuildOrderKey(directory->m_security.m_security, side,
      orderId);
    m_marketDataFeedClient->OffsetOrderSize(orderKey, -executedQuantity,
      timestamp);
    auto orderEntry = Beam::Retrieve(m_orderEntries, orderKey);
    if(orderEntry.is_initialized()) {
      orderEntry->m_remainingQuantity -= executedQuantity;
      if(m_config.m_isTimeAndSaleFeed) {
        auto condition = TimeAndSale::Condition();
        condition.m_code = "@";
        auto timeAndSale = TimeAndSale(timestamp, orderEntry->m_price,
          executedQuantity, std::move(condition),
          m_config.m_market.m_displayName);
        m_marketDataFeedClient->PublishTimeAndSale(
          SecurityTimeAndSale(std::move(timeAndSale),
          directory->m_security.m_security));
      }
      UpdateBbo(directory->m_security.m_security, side, orderEntry->m_price,
        -executedQuantity, timestamp);
    }
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::HandleOrderExecutedAtPriceMessage(
      const MoldUdp64::MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(Beam::Store(cursor));
    auto orderId = ParseInt64(Beam::Store(cursor));
    auto orderBookId = ParseInt32(Beam::Store(cursor));
    auto directory = Beam::Retrieve(m_orderBookDirectories, orderBookId);
    if(!directory.is_initialized()) {
      return;
    }
    auto side = ParseSide(Beam::Store(cursor));
    auto executedQuantity = static_cast<std::int64_t>(
      ParseInt64(Beam::Store(cursor)));
    auto matchId = ParseAlpha(12, Beam::Store(cursor));
    auto ownerMpid = ParseAlpha(7, Beam::Store(cursor));
    auto counterMpid = ParseAlpha(7, Beam::Store(cursor));
    auto price = ParsePrice(*directory, Beam::Store(cursor));
    auto atCross = ParseChar(Beam::Store(cursor));
    auto printable = ParseChar(Beam::Store(cursor));
    auto orderKey = BuildOrderKey(directory->m_security.m_security, side,
      orderId);
    m_marketDataFeedClient->OffsetOrderSize(orderKey, -executedQuantity,
      timestamp);
    auto orderEntry = Beam::Retrieve(m_orderEntries, orderKey);
    if(orderEntry.is_initialized()) {
      orderEntry->m_remainingQuantity -= executedQuantity;
      if(printable == 'Y' && m_config.m_isTimeAndSaleFeed) {
        auto condition = TimeAndSale::Condition();
        condition.m_code = "@";
        auto timeAndSale = TimeAndSale(timestamp, price, executedQuantity,
          std::move(condition), m_config.m_market.m_displayName);
        m_marketDataFeedClient->PublishTimeAndSale(
          SecurityTimeAndSale(std::move(timeAndSale),
          directory->m_security.m_security));
      }
      UpdateBbo(directory->m_security.m_security, side, orderEntry->m_price,
        -executedQuantity, timestamp);
    }
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::HandleOrderReplaceMessage(
      const MoldUdp64::MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(Beam::Store(cursor));
    auto orderId = ParseInt64(Beam::Store(cursor));
    auto orderBookId = ParseInt32(Beam::Store(cursor));
    auto directory = Beam::Retrieve(m_orderBookDirectories, orderBookId);
    if(!directory.is_initialized()) {
      return;
    }
    auto side = ParseSide(Beam::Store(cursor));
    auto newPosition = ParseInt32(Beam::Store(cursor));
    auto quantity = ParseInt64(Beam::Store(cursor));
    auto price = ParsePrice(*directory, Beam::Store(cursor));
    auto type = ParseInt16(Beam::Store(cursor));
    auto orderKey = BuildOrderKey(directory->m_security.m_security, side,
      orderId);
    m_marketDataFeedClient->DeleteOrder(orderKey, timestamp);
    auto orderEntry = Beam::Retrieve(m_orderEntries, orderKey);
    if(!orderEntry.is_initialized()) {
      return;
    }
    UpdateBbo(directory->m_security.m_security, side, orderEntry->m_price,
      -orderEntry->m_remainingQuantity, timestamp);
    auto newOrderEntry = *orderEntry;
    newOrderEntry.m_price = price;
    newOrderEntry.m_remainingQuantity = quantity;
    m_orderEntries[orderKey] = newOrderEntry;
    m_marketDataFeedClient->AddOrder(directory->m_security.m_security,
      m_config.m_market.m_displayName, newOrderEntry.m_mpid, false, orderKey,
      side, price, quantity, timestamp);
    UpdateBbo(directory->m_security.m_security, side, price, quantity,
      timestamp);
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::HandleOrderDeleteMessage(
      const MoldUdp64::MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(Beam::Store(cursor));
    auto orderId = ParseInt64(Beam::Store(cursor));
    auto orderBookId = ParseInt32(Beam::Store(cursor));
    auto directory = Beam::Retrieve(m_orderBookDirectories, orderBookId);
    if(!directory.is_initialized()) {
      return;
    }
    auto side = ParseSide(Beam::Store(cursor));
    auto orderKey = BuildOrderKey(directory->m_security.m_security, side,
      orderId);
    m_marketDataFeedClient->DeleteOrder(orderKey, timestamp);
    auto orderEntry = Beam::Retrieve(m_orderEntries, orderKey);
    if(!orderEntry.is_initialized()) {
      return;
    }
    UpdateBbo(directory->m_security.m_security, side, orderEntry->m_price,
      -orderEntry->m_remainingQuantity, timestamp);
    m_orderEntries.erase(orderKey);
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::HandleTradeMessage(
      const MoldUdp64::MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto timestamp = ParseTimestamp(Beam::Store(cursor));
    cursor += 12;
    auto side = ParseSide(Beam::Store(cursor));
    auto quantity = ParseInt64(Beam::Store(cursor));
    auto orderBookId = ParseInt32(Beam::Store(cursor));
    auto directory = Beam::Retrieve(m_orderBookDirectories, orderBookId);
    if(!directory.is_initialized()) {
      return;
    }
    auto price = ParsePrice(*directory, Beam::Store(cursor));
    auto ownerMpid = ParseAlpha(7, Beam::Store(cursor));
    auto counterMpid = ParseAlpha(7, Beam::Store(cursor));
    auto printable = ParseChar(Beam::Store(cursor));
    auto atCross = ParseChar(Beam::Store(cursor));
    if(printable == 'Y' && m_config.m_isTimeAndSaleFeed) {
      auto condition = TimeAndSale::Condition();
      condition.m_code = "@";
      auto timeAndSale = TimeAndSale(timestamp, price, quantity,
        std::move(condition), m_config.m_market.m_displayName);
      m_marketDataFeedClient->PublishTimeAndSale(
        SecurityTimeAndSale(std::move(timeAndSale),
        directory->m_security.m_security));
    }
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::HandleOrderBookDirectoryMessage(
      const MoldUdp64::MoldUdp64Message& message) {
    auto cursor = message.m_data;
    auto directory = OrderBookDirectory();
    auto timestamp = ParseTimestamp(Beam::Store(cursor));
    directory.m_id = ParseInt32(Beam::Store(cursor));
    auto symbol = ParseAlpha(32, Beam::Store(cursor));
    directory.m_security.m_security = Security(symbol, m_config.m_market.m_code,
      m_config.m_market.m_countryCode);
    directory.m_security.m_name = ParseAlpha(32, Beam::Store(cursor));
    auto isin = ParseAlpha(12, Beam::Store(cursor));
    auto productType = ParseInt8(Beam::Store(cursor));
    if(productType == 1) {
      directory.m_productType = ProductType::OPTION;
    } else if(productType == 3) {
      directory.m_productType = ProductType::FUTURE;
    } else if(productType == 5) {
      directory.m_productType = ProductType::EQUITY;
    }
    directory.m_currency = m_currencyDatabase->FromCode(
      ParseAlpha(3, Beam::Store(cursor))).m_id;
    directory.m_priceDecimalPlaces = ParseInt16(Beam::Store(cursor));
    directory.m_valueDecimalPlaces = ParseInt16(Beam::Store(cursor));
    directory.m_oddLotSize = ParseInt32(Beam::Store(cursor));
    directory.m_security.m_boardLot = ParseInt32(Beam::Store(cursor));
    directory.m_blockLotSize = ParseInt64(Beam::Store(cursor));
    if(directory.m_productType != ProductType::EQUITY) {
      return;
    }
    m_orderBookDirectories[directory.m_id] = directory;
    m_marketDataFeedClient->Add(directory.m_security);
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::Dispatch(
      const MoldUdp64::MoldUdp64Message& message) {
    if(message.m_messageType == 'T') {
      HandleSecondsMessage(message);
    } else if(message.m_messageType == 'A') {
      HandleAddOrderMessage(true, message);
    } else if(message.m_messageType == 'F') {
      HandleAddOrderMessage(false, message);
    } else if(message.m_messageType == 'E') {
      HandleOrderExecutedMessage(message);
    } else if(message.m_messageType == 'C') {
      HandleOrderExecutedAtPriceMessage(message);
    } else if(message.m_messageType == 'U') {
      HandleOrderReplaceMessage(message);
    } else if(message.m_messageType == 'D') {
      HandleOrderDeleteMessage(message);
    } else if(message.m_messageType == 'P') {
      HandleTradeMessage(message);
    } else if(message.m_messageType == 'R') {
      HandleOrderBookDirectoryMessage(message);
    }
  }

  template<typename M, typename I, typename G>
  void AsxItchMarketDataFeedClient<M, I, G>::ReadLoop() {
    auto lastSequenceNumber = std::uint64_t(-1);
    while(true) {
      auto packet = SoupBinTcp::SoupBinTcpPacket();
      try {
        packet = m_glimpseClient->Read();
      } catch(Beam::IO::NotConnectedException&) {
        break;
      } catch(Beam::IO::EndOfFileException&) {
        break;
      }
      if(packet.m_type == 'S') {
        auto message = MoldUdp64::MoldUdp64Message();
        message.m_length = packet.m_length - 1;
        message.m_messageType = packet.m_payload[0];
        message.m_data = &packet.m_payload[1];
        if(message.m_messageType == 'G') {
          auto cursor = message.m_data;
          lastSequenceNumber =
            SoupBinTcp::ParseLeftPaddedNumeric<std::uint64_t>(20,
            Beam::Store(cursor));
          break;
        }
        Dispatch(message);
      } else if(packet.m_type == 'Z') {
        break;
      }
    }
    m_glimpseClient->Close();
    while(true) {
      try {
        auto sequenceNumber = std::uint64_t();
        auto message = m_itchClient->Read(Beam::Store(sequenceNumber));
        if(lastSequenceNumber != -1 && sequenceNumber <= lastSequenceNumber) {
          continue;
        }
        if(lastSequenceNumber != -1 &&
            sequenceNumber > lastSequenceNumber + 1) {
          std::cout << "Packets dropped: " << (lastSequenceNumber + 1) <<
            " - " << (sequenceNumber - 1) << std::endl;
        }
        lastSequenceNumber = sequenceNumber;
        Dispatch(message);
      } catch(Beam::IO::NotConnectedException&) {
        break;
      } catch(Beam::IO::EndOfFileException&) {
        break;
      }
    }
  }
}

#endif
