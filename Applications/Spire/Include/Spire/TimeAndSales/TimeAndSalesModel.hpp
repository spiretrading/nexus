#ifndef SPIRE_TIME_AND_SALES_MODEL_HPP
#define SPIRE_TIME_AND_SALES_MODEL_HPP
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/TickerQuery.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/TimeAndSales/BboIndicator.hpp"

namespace Spire {
namespace Details {

  /** Stores a time and sale along with its BBO indicator. */
  struct TimeAndSalesEntry {

    /** The time and sale and its sequence. */
    Nexus::SequencedTimeAndSale m_time_and_sale;

    /** The BBO indicator of the time and sale. */
    BboIndicator m_indicator;

    bool operator ==(const TimeAndSalesEntry&) const = default;
  };
}

  /**
   * A list of a ticker's time and sales ordered from oldest to newest. New
   * time and sales are appended as they occur, older ones are inserted at the
   * front through load_older.
   */
  class TimeAndSalesModel : public ListModel<Details::TimeAndSalesEntry> {
    public:

      /**
       * Loads time and sales older than the first one at the front of the
       * list.
       * @param max_count The maximum number of time and sales to insert.
       * @return A promise that completes when the load completes.
       */
      virtual QtPromise<void> load_older(int max_count) = 0;

    protected:

      /** Constructs a TimeAndSalesModel. */
      TimeAndSalesModel() = default;
  };

  /**
   * Loads a ticker's time and sales over a range, waiting until the query
   * completes.
   * @param client The client used to submit the query.
   * @param ticker The ticker whose time and sales are loaded.
   * @param range The range of time and sales to load.
   * @param limit The limit on the number of time and sales to load.
   * @return The time and sales loaded, in sequence order.
   */
  std::vector<Nexus::SequencedTimeAndSale> load_time_and_sales(
    Nexus::MarketDataClient& client, const Nexus::Ticker& ticker,
    const Beam::Range& range, const Beam::SnapshotLimit& limit);

  /** The result of load_bbo_indicators. */
  struct BboIndicatorsLoadResult {

    /** The BboIndicator of each time and sale, in the same order. */
    std::vector<BboIndicator> m_indicators;

    /** The BBO quote at the last time and sale, or none if there is none. */
    boost::optional<Nexus::SequencedBboQuote> m_bbo;
  };

  /**
   * Loads the BboIndicator of each time and sale relative to the BBO quote at
   * it.
   * @param client The client used to load BBO quotes.
   * @param ticker The ticker the time and sales belong to.
   * @param time_and_sales The time and sales, in sequence order, to load
   *        indicators for.
   * @param initial_bbo The BBO quote before the first time and sale, or none
   *        if unknown. Passing the BBO quote of a previous call's result for
   *        the preceding time and sales avoids loading it again.
   * @return The BboIndicator of each time and sale, along with the BBO quote
   *         at the last one, which is <i>initial_bbo</i> if
   *         <i>time_and_sales</i> is empty.
   */
  BboIndicatorsLoadResult load_bbo_indicators(
    Nexus::MarketDataClient& client, const Nexus::Ticker& ticker,
    const std::vector<Nexus::SequencedTimeAndSale>& time_and_sales,
    boost::optional<Nexus::SequencedBboQuote> initial_bbo);
}

#endif
