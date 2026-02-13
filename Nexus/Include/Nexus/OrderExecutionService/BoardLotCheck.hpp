#ifndef NEXUS_BOARD_LOT_CHECK_HPP
#define NEXUS_BOARD_LOT_CHECK_HPP
#include <memory>
#include <type_traits>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckException.hpp"
#include "Nexus/TechnicalAnalysis/StandardTickerQueries.hpp"

namespace Nexus {

  /**
   * Validates an Order's board lot size.
   * @param <C> The type of MarketDataClient used to determine the price of a
   *        Ticker.
   */
  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  class BoardLotCheck : public OrderSubmissionCheck {
    public:

      /**
       * The type of MarketDataClient used to price Orders for buying power
       * checks.
       */
      using MarketDataClient = Beam::dereference_t<C>;

      /**
       * Constructs a BoardLotCheck.
       * @param market_data_client Initializes the MarketDataClient.
       * @param venues The available venues to submit an order to.
       * @param time_zones The database of timezones.
       */
      template<Beam::Initializes<C> CF>
      BoardLotCheck(CF&& market_data_client, VenueDatabase venues,
        boost::local_time::tz_database time_zones);

      void submit(const OrderInfo& info) override;

    private:
      struct ClosingEntry {
        boost::posix_time::ptime m_last_update;
        boost::optional<Money> m_closing_price;

        ClosingEntry();
      };
      Beam::local_ptr_t<C> m_market_data_client;
      VenueDatabase m_venues;
      boost::local_time::tz_database m_time_zones;
      Beam::SynchronizedUnorderedMap<
        Ticker, Beam::Sync<ClosingEntry, Beam::Mutex>> m_closing_entries;
      Beam::SynchronizedUnorderedMap<
        Ticker, std::shared_ptr<Beam::StateQueue<BboQuote>>> m_bbo_quotes;
      std::vector<Beam::RoutineHandler> m_query_routines;

      Money load_price(
        const Ticker& ticker, boost::posix_time::ptime timestamp);
  };

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  auto make_board_lot_check(C&& market_data_client,
      VenueDatabase venues, boost::local_time::tz_database time_zones) {
    return std::make_unique<BoardLotCheck<std::remove_reference_t<C>>>(
      std::forward<C>(market_data_client), std::move(venues),
      std::move(time_zones));
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  BoardLotCheck<C>::ClosingEntry::ClosingEntry()
    : m_last_update(boost::posix_time::neg_infin) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  BoardLotCheck<C>::BoardLotCheck(CF&& market_data_client, VenueDatabase venues,
    boost::local_time::tz_database time_zones)
    : m_market_data_client(std::forward<CF>(market_data_client)),
      m_venues(std::move(venues)),
      m_time_zones(std::move(time_zones)) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void BoardLotCheck<C>::submit(const OrderInfo& info) {
    if(info.m_fields.m_ticker.get_venue() != DefaultVenues::TSX &&
        info.m_fields.m_ticker.get_venue() != DefaultVenues::TSXV &&
        info.m_fields.m_ticker.get_venue() != DefaultVenues::NEOE &&
        info.m_fields.m_ticker.get_venue() != DefaultVenues::CSE) {
      return;
    }
    auto current_price = load_price(info.m_fields.m_ticker, info.m_timestamp);
    if(current_price < 10 * Money::CENT) {
      if(info.m_fields.m_quantity % 1000 != 0) {
        boost::throw_with_location(OrderSubmissionCheckException(
          "Quantity must be a multiple of 1000."));
      }
    } else if(current_price < Money::ONE) {
      if(info.m_fields.m_quantity % 500 != 0) {
        boost::throw_with_location(
          OrderSubmissionCheckException("Quantity must be a multiple of 500."));
      }
    } else {
      if(info.m_fields.m_quantity % 100 != 0) {
        boost::throw_with_location(
          OrderSubmissionCheckException("Quantity must be a multiple of 100."));
      }
    }
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  Money BoardLotCheck<C>::load_price(
      const Ticker& ticker, boost::posix_time::ptime timestamp) {
    auto& closing_entry = m_closing_entries.get(ticker);
    auto closing_price = Beam::with(closing_entry, [&] (auto& entry) {
      if(timestamp - entry.m_last_update > boost::posix_time::hours(1)) {
        if(auto close = load_previous_close(*m_market_data_client, ticker,
            timestamp, m_venues, m_time_zones)) {
          entry.m_closing_price = close->m_price;
          entry.m_last_update = timestamp;
        }
      }
      return entry.m_closing_price;
    });
    if(closing_price) {
      return *closing_price;
    }
    auto publisher = m_bbo_quotes.get_or_insert(ticker, [&] {
      auto publisher = std::make_shared<Beam::StateQueue<BboQuote>>();
      m_query_routines.push_back(query_real_time_with_snapshot(
        *m_market_data_client, ticker, publisher));
      return publisher;
    });
    try {
      auto effective_closing_price = publisher->peek().m_bid.m_price;
      return Beam::with(closing_entry, [&] (auto& entry) {
        entry.m_closing_price = effective_closing_price;
        entry.m_last_update = timestamp;
        return effective_closing_price;
      });
    } catch(const Beam::PipeBrokenException&) {
      m_bbo_quotes.erase(ticker);
      boost::throw_with_location(
        OrderSubmissionCheckException("No BBO quote available."));
    }
  }
}

#endif
