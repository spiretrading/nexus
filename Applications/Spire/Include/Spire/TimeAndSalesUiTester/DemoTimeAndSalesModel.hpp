#ifndef SPIRE_DEMO_TIME_AND_SALES_MODEL_HPP
#define SPIRE_DEMO_TIME_AND_SALES_MODEL_HPP
#include <vector>
#include <QTimer>
#include <Beam/TimeService/LiveTimer.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /*
   * Implements a TimeAndSalesModel model that periodically produces new
   * time and sales for demonstration purposes.
   */
  class DemoTimeAndSalesModel : public TimeAndSalesModel {
    public:

      /** Constructs a default DemoTimeAndSalesModel. */
      DemoTimeAndSalesModel();

      ~DemoTimeAndSalesModel() override;

      /* Returns the price of new time and sales. */
      Nexus::Money get_price() const;

      /* Sets the price of new time and sales. */
      void set_price(Nexus::Money price);

      /* Returns the bbo indicator. */
      BboIndicator get_bbo_indicator() const;

      /* Sets the bbo indicator. */
      void set_bbo_indicator(BboIndicator indicator);

      /* Returns the period for producing new time and sales. */
      boost::posix_time::time_duration get_period() const;

      /* Sets the period for producing new time and sales. */
      void set_period(boost::posix_time::time_duration period);

      /* Returns the amount of time it takes to query time and sales. */
      boost::posix_time::time_duration get_query_duration() const;

      /* Sets the amount of time needed to query time and sales. */
      void set_query_duration(boost::posix_time::time_duration duration);

      /** Returns <code>true</code> iff data is randomly generated. */
      bool is_data_random() const;

      /** Sets whether data is randomly generated. */
      void set_data_random(bool is_random);

      QtPromise<std::vector<Entry>> query_until(
        Beam::Sequence sequence, int max_count) override;

      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      Nexus::Money m_price;
      BboIndicator m_indicator;
      boost::posix_time::time_duration m_period;
      boost::posix_time::time_duration m_query_duration;
      bool m_is_data_random;
      QTimer m_timer;
      std::vector<std::shared_ptr<Beam::LiveTimer>> m_query_duration_timers;

      Entry make_entry(boost::posix_time::ptime timestamp) const;
      void on_timeout();
  };
}

#endif
