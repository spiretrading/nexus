#ifndef SPIRE_NONE_TIME_AND_SALES_MODEL_HPP
#define SPIRE_NONE_TIME_AND_SALES_MODEL_HPP
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /* Implements a none TimeAndSalesModel. */
  class NoneTimeAndSalesModel : public TimeAndSalesModel {
    public:

      /* Constructs a NoneTimeAndSalesModel. */
      explicit NoneTimeAndSalesModel(Nexus::Security security);

      const Nexus::Security& get_security() const override;

      QtPromise<std::vector<Entry>> query_until(
        Beam::Queries::Sequence sequence, int max_count) override;

      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      Nexus::Security m_security;
  };
}

#endif
