#ifndef SPIRE_NONE_TIME_AND_SALES_MODEL_HPP
#define SPIRE_NONE_TIME_AND_SALES_MODEL_HPP
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /* Implements a none TimeAndSalesModel. */
  class NoneTimeAndSalesModel : public TimeAndSalesModel {
    public:
      QtPromise<void> load_older(int max_count) override;
      int get_size() const override;
      const Type& get(int index) const override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;
  };
}

#endif
