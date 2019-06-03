#ifndef SPIRE_EMPTY_TIME_AND_SALES_MODEL_HPP
#define SPIRE_EMPTY_TIME_AND_SALES_MODEL_HPP
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  //! Implements the TimeAndSalesModel with no entries and no updates.
  class EmptyTimeAndSalesModel final : public TimeAndSalesModel {
    public:

      //! Constructs an empty model.
      /*!
        \param security The security to model.
      */
      explicit EmptyTimeAndSalesModel(Nexus::Security security);

      const Nexus::Security& get_security() const override;

      Nexus::Quantity get_volume() const override;

      QtPromise<std::vector<Entry>> load_snapshot(Beam::Queries::Sequence last,
        int count) override;

      boost::signals2::connection connect_time_and_sale_signal(
        const TimeAndSaleSignal::slot_type& slot) const override;

      boost::signals2::connection connect_volume_signal(
        const VolumeSignal::slot_type& slot) const override;

    private:
      Nexus::Security m_security;
  };
}

#endif
