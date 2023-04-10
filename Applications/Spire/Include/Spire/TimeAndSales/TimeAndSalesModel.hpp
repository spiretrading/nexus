#ifndef SPIRE_TIME_AND_SALES_MODEL_HPP
#define SPIRE_TIME_AND_SALES_MODEL_HPP
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/TimeAndSales/BboIndicator.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"

namespace Spire {

  /** Used to retreive time and sales for a security. */
  class TimeAndSalesModel {
    public:

      /** Represents a time and sale entry. */
      struct Entry {

        /** The time and sale. */
        Nexus::SequencedTimeAndSale m_time_and_sale;

        /** The BBO indicator that the entry belongs to. */
        BboIndicator m_indicator;
      };

      /**
       * Signals a new time and sale.
       * @param entry A new time and sale.
       */
      using UpdateSignal = Signal<void (const Entry& entry)>;

      virtual ~TimeAndSalesModel() = default;

      /** Returns the security that the model uses. */
      virtual const Nexus::Security& get_security() const = 0;

      /*
       * Query the limited number of entries before the specified sequence.
       * @param sequence The end sequence number that the query should stop.
       * @param max_count The maximum number of entries to query.
       * @return A list of time and sales no greater than <i>max_count</i> items
       *         where the last item's sequence number is no greater than
       *         <i>sequence</i>.
      */
      virtual QtPromise<std::vector<Entry>> query_until(
        Beam::Queries::Sequence sequence, int max_count) = 0;

      /* Connects a slot to the update signal. */
      virtual boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const = 0;

    protected:

      /* Constructs a TimeAndSalesModel. */
      TimeAndSalesModel() = default;

    private:
      TimeAndSalesModel(const TimeAndSalesModel&) = delete;
      TimeAndSalesModel& operator =(const TimeAndSalesModel&) = delete;
  };
}

#endif
