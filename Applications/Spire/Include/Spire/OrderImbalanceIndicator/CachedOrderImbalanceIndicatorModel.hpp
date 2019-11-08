#ifndef SPIRE_CACHED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_CACHED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

namespace Spire {

  //! Represents an OrderImbalanceModel that caches loaded order imbalances
  //! from a source model.
  class CachedOrderImbalanceIndicatorModel :
      public OrderImbalanceIndicatorModel {
    public:

      //! Constructs a CachedOrderImbalanceModel with a given source model.
      /*
        \param source_model The model supplying data to the cached model.
      */
      CachedOrderImbalanceIndicatorModel(
        std::shared_ptr<OrderImbalanceIndicatorModel> source_model);

      std::tuple<boost::signals2::connection,
        QtPromise<std::vector<Nexus::OrderImbalance>>>
        subscribe(const boost::posix_time::ptime& start,
          const boost::posix_time::ptime& end,
          const OrderImbalanceSignal::slot_type& slot) override;

    private:
      std::shared_ptr<OrderImbalanceIndicatorModel> m_source_model;
      std::vector<Nexus::OrderImbalance> m_imbalances;
  };
}

#endif
