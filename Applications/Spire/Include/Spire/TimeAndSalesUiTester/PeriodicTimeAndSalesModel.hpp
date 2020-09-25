#ifndef SPIRE_PERIODIC_TIME_AND_SALES_MODEL_HPP
#define SPIRE_PERIODIC_TIME_AND_SALES_MODEL_HPP
#include <atomic>
#include <Beam/Threading/Threading.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <QTimer>
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  //! Implements a time and sales model that periodically emits new values.
  class PeriodicTimeAndSalesModel final : public TimeAndSalesModel,
      public QObject {
    public:

      //! Constructs an empty model.
      /*!
        \param security The security to model.
        \param timer_thread_pool The timer pool used to simulating load times.
      */
      PeriodicTimeAndSalesModel(Nexus::Security security);

      //! Returns the price of new time and sales.
      Nexus::Money get_price() const;

      //! Sets the price of new time and sales.
      void set_price(Nexus::Money price);

      //! Returns the price range used to emit time and sales.
      TimeAndSalesProperties::PriceRange get_price_range() const;

      //! Sets the price range used to emit time and sales.
      void set_price_range(TimeAndSalesProperties::PriceRange r);

      //! Returns the amount of time it takes to load a snapshot.
      boost::posix_time::time_duration get_load_duration() const;

      //! Sets the amount of time needed to load a snapshot.
      void set_load_duration(boost::posix_time::time_duration d);

      //! Returns the model's period.
      boost::posix_time::time_duration get_period() const;

      //! Sets the model's period.
      void set_period(boost::posix_time::time_duration p);

      const Nexus::Security& get_security() const override;

      Nexus::Quantity get_volume() const override;

      QtPromise<std::vector<Entry>> load_snapshot(Beam::Queries::Sequence last,
        int count) override;

      boost::signals2::connection connect_time_and_sale_signal(
        const TimeAndSaleSignal::slot_type& slot) const override;

      boost::signals2::connection connect_volume_signal(
        const VolumeSignal::slot_type& slot) const override;

    private:
      mutable TimeAndSaleSignal m_time_and_sale_signal;
      mutable VolumeSignal m_volume_signal;
      Nexus::Security m_security;
      Nexus::Money m_price;
      TimeAndSalesProperties::PriceRange m_price_range;
      boost::posix_time::time_duration m_load_duration;
      boost::posix_time::time_duration m_period;
      Nexus::Quantity m_volume;
      std::shared_ptr<std::atomic_bool> m_is_loaded;
      std::vector<Entry> m_entries;
      QTimer m_timer;

      void on_timeout();
  };
}

#endif
