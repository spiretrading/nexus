#ifndef SPIRE_LOCAL_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_LOCAL_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

namespace Spire {

  //! Represents an OrderImbalanceIndicatorModel that accepts insertion of
  //! new order imbalances.
  class LocalOrderImbalanceIndicatorModel :
      public OrderImbalanceIndicatorModel {
    public:

      std::tuple<boost::signals2::connection,
        QtPromise<std::vector<Nexus::OrderImbalance>>>
          subscribe(const boost::posix_time::ptime& start,
            const boost::posix_time::ptime& end,
            const OrderImbalanceSignal::slot_type& slot) override;

      //! Inserts an order imbalance into the model.
      /*
        \param imbalance The order imbalance to insert into the model.
      */
      void insert(const Nexus::OrderImbalance& imbalance);

    private:
      std::vector<OrderImbalanceSignalConnection> m_signals;
      std::vector<Nexus::OrderImbalance> m_imbalances;
  };
}

#endif
