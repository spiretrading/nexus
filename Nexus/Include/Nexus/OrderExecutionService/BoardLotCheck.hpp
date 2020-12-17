#ifndef NEXUS_BOARD_LOT_CHECK_HPP
#define NEXUS_BOARD_LOT_CHECK_HPP
#include <memory>
#include <type_traits>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckException.hpp"
#include "Nexus/TechnicalAnalysis/StandardSecurityQueries.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Validates an Order's board lot size.
   * @param <C> The type of MarketDataClient used to determine the price of a
   *        Security.
   */
  template<typename C>
  class BoardLotCheck : public OrderSubmissionCheck {
    public:

      /**
       * The type of MarketDataClient used to price Orders for buying power
       * checks.
       */
      using MarketDataClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a BoardLotCheck.
       * @param marketDataClient Initializes the MarketDataClient.
       */
      template<typename CF>
      BoardLotCheck(CF&& marketDataClient, const MarketDatabase& marketDatabase,
        const boost::local_time::tz_database& timeZoneDatabase);

      void Submit(const OrderInfo& orderInfo) override;

    private:
      struct ClosingEntry {
        boost::posix_time::ptime m_lastUpdate;
        boost::optional<Money> m_closingPrice;

        ClosingEntry();
      };
      Beam::GetOptionalLocalPtr<C> m_marketDataClient;
      MarketDatabase m_marketDatabase;
      boost::local_time::tz_database m_timeZoneDatabase;
      Beam::SynchronizedUnorderedMap<Security,
        Beam::Threading::Sync<ClosingEntry, Beam::Threading::Mutex>>
          m_closingEntries;
      Beam::SynchronizedUnorderedMap<Security,
        std::shared_ptr<Beam::StateQueue<BboQuote>>> m_bboQuotes;

      Money LoadPrice(const Security& security,
        boost::posix_time::ptime timestamp);
  };

  template<typename C>
  BoardLotCheck<C>::ClosingEntry::ClosingEntry()
    : m_lastUpdate(boost::posix_time::neg_infin),
      m_closingPrice(Money::ZERO) {}

  template<typename MarketDataClient>
  auto MakeBoardLotCheck(MarketDataClient&& marketDataClient,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    return std::make_unique<BoardLotCheck<std::decay_t<MarketDataClient>>>(
      std::forward<MarketDataClient>(marketDataClient), marketDatabase,
      timeZoneDatabase);
  }

  template<typename C>
  template<typename CF>
  BoardLotCheck<C>::BoardLotCheck(CF&& marketDataClient,
    const MarketDatabase& marketDatabase,
    const boost::local_time::tz_database& timeZoneDatabase)
    : m_marketDataClient(std::forward<CF>(marketDataClient)),
      m_marketDatabase(marketDatabase),
      m_timeZoneDatabase(timeZoneDatabase) {}

  template<typename C>
  void BoardLotCheck<C>::Submit(const OrderInfo& orderInfo) {
    if(orderInfo.m_fields.m_security.GetMarket() != DefaultMarkets::TSX() &&
        orderInfo.m_fields.m_security.GetMarket() != DefaultMarkets::TSXV() &&
        orderInfo.m_fields.m_security.GetMarket() != DefaultMarkets::NEOE() &&
        orderInfo.m_fields.m_security.GetMarket() != DefaultMarkets::CSE()) {
      return;
    }
    auto currentPrice = LoadPrice(orderInfo.m_fields.m_security,
      orderInfo.m_timestamp);
    if(currentPrice < 10 * Money::CENT) {
      if(orderInfo.m_fields.m_quantity % 1000 != 0) {
        BOOST_THROW_EXCEPTION(OrderSubmissionCheckException(
          "Quantity must be a multiple of 1000."));
      }
    } else if(currentPrice < Money::ONE) {
      if(orderInfo.m_fields.m_quantity % 500 != 0) {
        BOOST_THROW_EXCEPTION(OrderSubmissionCheckException(
          "Quantity must be a multiple of 500."));
      }
    } else {
      if(orderInfo.m_fields.m_quantity % 100 != 0) {
        BOOST_THROW_EXCEPTION(OrderSubmissionCheckException(
          "Quantity must be a multiple of 100."));
      }
    }
  }

  template<typename C>
  Money BoardLotCheck<C>::LoadPrice(const Security& security,
      boost::posix_time::ptime timestamp) {
    auto& closingEntry = m_closingEntries.Get(security);
    auto closingPrice = Beam::Threading::With(closingEntry, [&] (auto& entry) {
      if(timestamp - entry.m_lastUpdate > boost::posix_time::hours(1)) {
        entry.m_closingPrice = TechnicalAnalysis::LoadPreviousClose(
          *m_marketDataClient, security, timestamp, m_marketDatabase,
          m_timeZoneDatabase);
        entry.m_lastUpdate = timestamp;
      }
      return entry.m_closingPrice;
    });
    if(closingPrice) {
      return *closingPrice;
    }
    auto publisher = m_bboQuotes.GetOrInsert(security, [&] {
      auto publisher = std::make_shared<Beam::StateQueue<BboQuote>>();
      MarketDataService::QueryRealTimeWithSnapshot(security,
        *m_marketDataClient, publisher);
      return publisher;
    });
    try {
      auto effectiveClosingPrice = publisher->Peek().m_bid.m_price;
      return Beam::Threading::With(closingEntry, [&] (auto& entry) {
        entry.m_closingPrice = effectiveClosingPrice;
        entry.m_lastUpdate = timestamp;
        return entry.m_closingPrice;
      });
    } catch(const Beam::PipeBrokenException&) {
      m_bboQuotes.Erase(security);
      BOOST_THROW_EXCEPTION(OrderSubmissionCheckException(
        "No BBO quote available."));
    }
  }
}

#endif
