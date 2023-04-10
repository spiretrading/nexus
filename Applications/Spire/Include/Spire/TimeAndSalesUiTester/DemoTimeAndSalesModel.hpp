#ifndef SPIRE_DEMO_TIME_AND_SALES_MODEL_HPP
#define SPIRE_DEMO_TIME_AND_SALES_MODEL_HPP
#include <QTimer>
#include "Nexus/Definitions/Money.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /*
   * Implements a TimeAndSalesModel model that periodically produces new
   * time and sales for demonstration purposes.
   */
  class DemoTimeAndSalesModel : public TimeAndSalesModel {
    public:

      /** Constructs a DemoTimeAndSalesModel. */
      explicit DemoTimeAndSalesModel(Nexus::Security security);

      /* Returns the price of new time and sales. */
      Nexus::Money get_price() const;

      /* Sets the price of new time and sales. */
      void set_price(Nexus::Money price);

      /* Returns the bbo indicator. */
      BboIndicator get_bbo_indicator() const;

      /* Sets the bbo indicator. */
      void set_bbo_indicator(BboIndicator indicator);

      /* Returns the model's period. */
      boost::posix_time::time_duration get_period() const;

      /* Sets the model's period. */
      void set_period(boost::posix_time::time_duration period);

      /* Returns the amount of time it takes to query time and sales. */
      boost::posix_time::time_duration get_query_duration() const;

      /* Sets the amount of time needed to query time and sales. */
      void set_query_duration(boost::posix_time::time_duration duration);

      const Nexus::Security& get_security() const override;

      QtPromise<std::vector<Entry>> query_until(
        Beam::Queries::Sequence sequence, int max_count) override;

      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      Nexus::Security m_security;
      Nexus::Money m_price;
      BboIndicator m_indicator;
      boost::posix_time::time_duration m_period;
      boost::posix_time::time_duration m_query_duration;
      QTimer m_timer;

      void on_timeout();
  };
}

#endif
