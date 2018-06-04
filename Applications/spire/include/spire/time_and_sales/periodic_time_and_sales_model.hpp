#ifndef SPIRE_PERIODIC_TIME_AND_SALES_MODEL_HPP
#define SPIRE_PERIODIC_TIME_AND_SALES_MODEL_HPP
#include <QTimer>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
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
      */
      periodic_time_and_sales_model(Nexus::Security s);

      //! Returns the price of new time and sales.
      Nexus::Money get_price() const;

      //! Sets the price of new time and sales.
      void set_price(Nexus::Money price);

      //! Returns the price range used to emit time and sales.
      time_and_sales_properties::price_range get_price_range() const;

      //! Sets the price range used to emit time and sales.
      void set_price_range(time_and_sales_properties::price_range r);

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
      Nexus::Money m_price;
      time_and_sales_properties::price_range m_price_range;
      boost::posix_time::time_duration m_period;
      Nexus::Quantity m_volume;
      std::vector<entry> m_entries;
      QTimer m_timer;

      void on_timeout();
  };
}

#endif
