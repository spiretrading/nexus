#ifndef SPIRE_GENERATED_TIME_AND_SALES_MODEL_HPP
#define SPIRE_GENERATED_TIME_AND_SALES_MODEL_HPP
#include <QTimer>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /**
   * Implements a TimeAndSalesModel that deterministically generates one time
   * and sale per period for a ticker.
   */
  class GeneratedTimeAndSalesModel : public TimeAndSalesModel {
    public:

      /**
       * Constructs a GeneratedTimeAndSalesModel.
       * @param ticker The ticker to generate time and sales for.
       * @param start The timestamp of the first time and sale.
       * @param period The amount of time between two time and sales.
       */
      GeneratedTimeAndSalesModel(Nexus::Ticker ticker,
        boost::posix_time::ptime start,
        boost::posix_time::time_duration period);

      QtPromise<std::vector<Entry>> query_until(
        Beam::Sequence sequence, int max_count) override;

      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      Nexus::Ticker m_ticker;
      boost::posix_time::ptime m_start;
      boost::posix_time::time_duration m_period;
      int m_seed;
      double m_price;
      int m_index;
      QTimer m_timer;

      int get_index(boost::posix_time::ptime timestamp) const;
      Entry make_entry(int index) const;
      void on_timeout();
  };
}

#endif
