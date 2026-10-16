#ifndef SPIRE_TIME_AND_SALES_MODEL_HPP
#define SPIRE_TIME_AND_SALES_MODEL_HPP
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

}

#endif
