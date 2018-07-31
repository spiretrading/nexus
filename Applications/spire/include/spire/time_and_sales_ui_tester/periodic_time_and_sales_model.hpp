#ifndef SPIRE_PERIODIC_TIME_AND_SALES_MODEL_HPP
#define SPIRE_PERIODIC_TIME_AND_SALES_MODEL_HPP
#include <atomic>
#include <Beam/Threading/Threading.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <QTimer>
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_model.hpp"

namespace spire {

  //! Implements a time and sales model that periodically emits new values.
  class periodic_time_and_sales_model final : public time_and_sales_model,
      public QObject {
    public:

      //! Constructs an empty model.
      /*!
        \param s The security to model.
        \param timer_thread_pool The timer pool used to simulating load times.
      */
      periodic_time_and_sales_model(Nexus::Security s,
        Beam::Threading::TimerThreadPool& timer_thread_pool);

      //! Returns the price of new time and sales.
      Nexus::Money get_price() const;

      //! Sets the price of new time and sales.
      void set_price(Nexus::Money price);

      //! Returns the price range used to emit time and sales.
      time_and_sales_properties::price_range get_price_range() const;

      //! Sets the price range used to emit time and sales.
      void set_price_range(time_and_sales_properties::price_range r);

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

      qt_promise<std::vector<entry>> load_snapshot(Beam::Queries::Sequence last,
        int count) override;

      boost::signals2::connection connect_time_and_sale_signal(
        const time_and_sale_signal::slot_type& slot) const override;

      boost::signals2::connection connect_volume_signal(
        const volume_signal::slot_type& slot) const override;

    private:
      mutable time_and_sale_signal m_time_and_sale_signal;
      mutable volume_signal m_volume_signal;
      Nexus::Security m_security;
      Beam::Threading::TimerThreadPool* m_timer_thread_pool;
      Nexus::Money m_price;
      time_and_sales_properties::price_range m_price_range;
      boost::posix_time::time_duration m_load_duration;
      boost::posix_time::time_duration m_period;
      Nexus::Quantity m_volume;
      std::shared_ptr<std::atomic_bool> m_is_loaded;
      std::vector<entry> m_entries;
      QTimer m_timer;

      void on_timeout();
  };
}

#endif
