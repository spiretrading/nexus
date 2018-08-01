#ifndef SPIRE_EMPTY_TIME_AND_SALES_MODEL_HPP
#define SPIRE_EMPTY_TIME_AND_SALES_MODEL_HPP
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_model.hpp"

namespace spire {

  //! Implements a time and sales model with no entries and no updates.
  class empty_time_and_sales_model final : public time_and_sales_model {
    public:

      //! Constructs an empty model.
      /*!
        \param s The security to model.
      */
      empty_time_and_sales_model(Nexus::Security s);

      const Nexus::Security& get_security() const override;

      Nexus::Quantity get_volume() const override;

      QtPromise<std::vector<entry>> load_snapshot(Beam::Queries::Sequence last,
        int count) override;

      boost::signals2::connection connect_time_and_sale_signal(
        const time_and_sale_signal::slot_type& slot) const override;

      boost::signals2::connection connect_volume_signal(
        const volume_signal::slot_type& slot) const override;

    private:
      Nexus::Security m_security;
  };
}

#endif
