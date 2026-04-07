#ifndef SPIRE_NONE_TIME_AND_SALES_MODEL_HPP
#define SPIRE_NONE_TIME_AND_SALES_MODEL_HPP
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /* Implements a none TimeAndSalesModel. */
  class NoneTimeAndSalesModel : public TimeAndSalesModel {
    public:
      QtPromise<std::vector<Entry>> query_until(
        Beam::Sequence sequence, int max_count) override;
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;
  };
}

#endif
