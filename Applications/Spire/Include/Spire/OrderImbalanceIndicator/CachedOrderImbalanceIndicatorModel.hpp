#ifndef SPIRE_CACHED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_CACHED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include <boost/icl/interval_set.hpp>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

namespace Spire {

  //! Represents an OrderImbalanceModel that caches loaded order imbalances
  //! from a source model.
  class CachedOrderImbalanceIndicatorModel :
      public OrderImbalanceIndicatorModel {
    public:

      //! Constructs a CachedOrderImbalanceModel with a given source model.
      /*
        \param source The model supplying data to the cached model.
      */
      CachedOrderImbalanceIndicatorModel(
        std::shared_ptr<OrderImbalanceIndicatorModel> source);

      SubscriptionResult subscribe(boost::posix_time::ptime start,
        boost::posix_time::ptime end,
        const OrderImbalanceSignal::slot_type& slot) override;

    private:
      struct Subscription {
        OrderImbalanceSignal m_imbalance_signal;
        boost::posix_time::ptime m_start;
        boost::posix_time::ptime m_end;

        Subscription(boost::posix_time::ptime start,
          boost::posix_time::ptime end);
      };
      std::shared_ptr<OrderImbalanceIndicatorModel> m_source;
      std::vector<Nexus::OrderImbalance> m_imbalances;
      std::vector<Subscription> m_subscriptions;
      boost::icl::interval_set<boost::posix_time::ptime> m_ranges;
      std::vector<boost::signals2::scoped_connection> m_connections;

      SubscriptionResult make_subscription(boost::posix_time::ptime start,
        boost::posix_time::ptime end,
        const OrderImbalanceSignal::slot_type& slot);
      SubscriptionResult load_imbalances(boost::posix_time::ptime start,
        boost::posix_time::ptime end,
        const OrderImbalanceSignal::slot_type& slot);
      void on_order_imbalance(const Nexus::OrderImbalance& imbalance);
  };
}

#endif
